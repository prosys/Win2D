// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#include "pch.h"

namespace ABI { namespace Microsoft { namespace Graphics { namespace Canvas
{
    DefaultDeviceAdapter::DefaultDeviceAdapter()
    {
        HSTRING stringActivableClassId = 
            Wrappers::HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get();

        ThrowIfFailed(GetActivationFactory(stringActivableClassId, &m_propertyValueStatics));
    }

    //
    // This implementation of the adapter does the normal thing, and calls into
    // D2D. It is separated out so tests can inject their own implementation.
    //
    ComPtr<ID2D1Factory2> DefaultDeviceAdapter::CreateD2DFactory(
        CanvasDebugLevel debugLevel)
    {
        D2D1_FACTORY_OPTIONS factoryOptions;
        factoryOptions.debugLevel = static_cast<D2D1_DEBUG_LEVEL>(debugLevel);

        ComPtr<ID2D1Factory2> createdFactory;
        ThrowIfFailed(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_MULTI_THREADED, 
            __uuidof(ID2D1Factory2), 
            &factoryOptions, 
            &createdFactory));

        return createdFactory;
    }

    bool DefaultDeviceAdapter::TryCreateD3DDevice(
        bool useSoftwareRenderer,
        bool useDebugD3DDevice,
        ComPtr<ID3D11Device>* device)
    {
        D3D_DRIVER_TYPE driverType = useSoftwareRenderer ? D3D_DRIVER_TYPE_WARP : D3D_DRIVER_TYPE_HARDWARE;

        UINT deviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        if (useDebugD3DDevice)
        {
            deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
        }

        ComPtr<ID3D11Device> createdDevice;
        if (SUCCEEDED(D3D11CreateDevice(
            NULL, // adapter
            driverType,
            NULL, // software handle
            deviceFlags,
            NULL, // feature level array
            0,  // feature level count
            D3D11_SDK_VERSION,
            &createdDevice,
            NULL,
            NULL)))
        {
            *device = createdDevice;
            return true;
        }
        else
        {
            return false;
        }
    }

    ComPtr<IDXGIDevice3> DefaultDeviceAdapter::GetDxgiDevice(ID2D1Device1* d2dDevice)
    {
        //
        // We want to find the DXGI device associated with an ID2D1Device.
        //
        // On Win10, there is direct API support for doing this.
        //
        // On Win8, there isn't. However, it's possible to figure it out by 
        // creating a bitmap and querying that for the dxgi
        // surface and then its dxgi device.
        //
        ComPtr<IDXGIDevice3> dxgiDevice3;

#if WINVER > _WIN32_WINNT_WINBLUE
        auto d2dDevice2 = As<ID2D1Device2>(d2dDevice);

        ComPtr<IDXGIDevice> dxgiDevice;
        ThrowIfFailed(d2dDevice2->GetDxgiDevice(&dxgiDevice));

        ThrowIfFailed(dxgiDevice.As(&dxgiDevice3));
#else
        ComPtr<ID2D1DeviceContext> deviceContext;
        ThrowIfFailed(d2dDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &deviceContext));

        ComPtr<ID2D1Bitmap1> bitmap;
        ThrowIfFailed(deviceContext->CreateBitmap(
            D2D1_SIZE_U{ 1, 1 },
            nullptr,
            0,
            D2D1::BitmapProperties1(
                D2D1_BITMAP_OPTIONS_NONE,
                D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
            &bitmap));

        ComPtr<IDXGISurface> surface;
        ThrowIfFailed(bitmap->GetSurface(&surface));

        ThrowIfFailed(surface->GetDevice(IID_PPV_ARGS(&dxgiDevice3)));
#endif

        return dxgiDevice3;
    }


    ComPtr<ICoreApplication> DefaultDeviceAdapter::GetCoreApplication()
    {
        ComPtr<ICoreApplication> coreApplication;
        ThrowIfFailed(GetActivationFactory(
            HStringReference(RuntimeClass_Windows_ApplicationModel_Core_CoreApplication).Get(),
            &coreApplication));

        return coreApplication;
    }

    ComPtr<IPropertyValueStatics> DefaultDeviceAdapter::GetPropertyValueStatics()
    {
        return m_propertyValueStatics;
    }


    //
    // SharedDeviceState
    //

    void ThrowIfInvalid(CanvasDebugLevel debugLevel)
    {
        switch (debugLevel)
        {
        case CanvasDebugLevel::None:
        case CanvasDebugLevel::Warning:
        case CanvasDebugLevel::Error:
        case CanvasDebugLevel::Information:
            break;
        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    SharedDeviceState::SharedDeviceState()
        : m_adapter(CanvasDeviceAdapter::GetInstance())
    {
        m_debugLevel = LoadDebugLevelProperty();
    }

    SharedDeviceState::~SharedDeviceState()
    {
        // 
        // Commit the debug level property back into the application properties.
        //
        SaveDebugLevelProperty(m_debugLevel);
    }


    ComPtr<ICanvasDevice> SharedDeviceState::GetSharedDevice(
        bool forceSoftwareRenderer)
    {
        //
        // This code, unlike other non-control APIs, cannot rely on the D2D
        // API lock. SharedDeviceState keeps its own lock for this purpose.
        //
        Lock lock(m_mutex);

        int cacheIndex = forceSoftwareRenderer ? 1 : 0;
        assert(cacheIndex < _countof(m_sharedDevices));

        ComPtr<ICanvasDevice> device = LockWeakRef<ICanvasDevice>(m_sharedDevices[cacheIndex]);
        ComPtr<ICanvasDevice> lostDevice;

        if (device && FAILED(static_cast<CanvasDevice*>(device.Get())->GetDeviceRemovedErrorCode()))
        {
            // We need to raise DeviceLost when we detect a lost device, but we
            // can't do that while we're holding the lock, so we need to do it
            // later.
            lostDevice = device;
            device.Reset();
        }

        if (!device)
        {
            //
            // Any new devices we create here will honor the debug 
            // level set at the time.
            // But existing devices that we return do not need to match
            // the debug level.
            // This goes along with the debug level option not having
            // retro-active behavior, and the fact that we expect apps to set the 
            // debug level at start-up, before creating any devices.
            //
            device = CanvasDevice::CreateNew(forceSoftwareRenderer);
            m_sharedDevices[cacheIndex] = AsWeak(device.Get());
        }

        lock.unlock();

        // Now that the lock has been released we can safely raise DeviceLost
        if (lostDevice)
            lostDevice->RaiseDeviceLost();

        assert(device);
        return device;
    }

    SharedDeviceState::PropertyData SharedDeviceState::GetDebugLevelPropertyData()
    {
        auto coreApplication = m_adapter->GetCoreApplication();

        ComPtr<IPropertySet> properties;
        ThrowIfFailed(coreApplication->get_Properties(
            &properties));

        ComPtr<IMap<HSTRING, IInspectable*>> propertyMap;
        ThrowIfFailed(properties.As(&propertyMap));

        // This function's decorated name is simply used as a key.
        HStringReference keyName(TEXT(__FUNCDNAME__));
        ComPtr<IInspectable> inspectableDebugLevelPropertyHolder;

        HRESULT hr = propertyMap->Lookup(keyName.Get(), &inspectableDebugLevelPropertyHolder);

        PropertyData ret = { keyName, propertyMap, inspectableDebugLevelPropertyHolder, hr };
        return ret;
    }

    void SharedDeviceState::StoreValueToPropertyKey(PropertyData key, CanvasDebugLevel debugLevel)
    {
        uint32_t value = static_cast<uint32_t>(debugLevel);
        ComPtr<IPropertyValue> debugLevelPropertyHolder;
        ThrowIfFailed(m_adapter->GetPropertyValueStatics()->CreateUInt32(value, &debugLevelPropertyHolder));

        boolean inserted = false; // unused
        ThrowIfFailed(key.PropertyMap->Insert(key.KeyName.Get(), debugLevelPropertyHolder.Get(), &inserted));
    }

    CanvasDebugLevel SharedDeviceState::LoadDebugLevelProperty()
    {
        auto propertyData = GetDebugLevelPropertyData();

        if (SUCCEEDED(propertyData.LookupResult))
        {
            ComPtr<IPropertyValue> debugLevelPropertyHolder;
            debugLevelPropertyHolder = static_cast<IPropertyValue*>(propertyData.PropertyHolder.Get());

            uint32_t value;
            ThrowIfFailed(debugLevelPropertyHolder->GetUInt32(&value));
            return static_cast<CanvasDebugLevel>(value);
        }
        else
        {
            //
            // If we couldn't load the property, create a new one and initialize it to None.
            //
            StoreValueToPropertyKey(propertyData, CanvasDebugLevel::None);
            return CanvasDebugLevel::None;
        }
    }

    void SharedDeviceState::SaveDebugLevelProperty(CanvasDebugLevel debugLevel)
    {
        StoreValueToPropertyKey(GetDebugLevelPropertyData(), debugLevel);
    }

    CanvasDebugLevel SharedDeviceState::GetDebugLevel()
    {
        //
        // m_debugLevel requires synchronization. One option might be use the existing mutex
        // in SharedDeviceState- except GetSharedDevice already uses that lock to control
        // cached devices, and the new device construction path also needs to look up the
        // debug level. 
        //
        // Rather than complicate things to accomodate this, we use a
        // std::atomic member to do this simple synchronization.
        //
        auto ret = m_debugLevel.load();

        return ret;
    }

    void SharedDeviceState::SetDebugLevel(CanvasDebugLevel const& value)
    {
        m_debugLevel.store(value);
    }


    //
    // CanvasDeviceFactory
    //

    IFACEMETHODIMP CanvasDeviceFactory::CreateWithForceSoftwareRendererOption(
        boolean forceSoftwareRenderer,
        ICanvasDevice** canvasDevice)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckAndClearOutPointer(canvasDevice);

                auto newCanvasDevice = CanvasDevice::CreateNew(!!forceSoftwareRenderer);
                
                ThrowIfFailed(newCanvasDevice.CopyTo(canvasDevice));
            });
    }

    IFACEMETHODIMP CanvasDeviceFactory::CreateFromDirect3D11Device(
        IDirect3DDevice* direct3DDevice,
        ICanvasDevice** canvasDevice)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckInPointer(direct3DDevice);
                CheckAndClearOutPointer(canvasDevice);

                auto newCanvasDevice = CanvasDevice::CreateNew(direct3DDevice);

                ThrowIfFailed(newCanvasDevice.CopyTo(canvasDevice));
            });
    }

    _Use_decl_annotations_
        IFACEMETHODIMP CanvasDeviceFactory::ActivateInstance(IInspectable **object)
    {
        return ExceptionBoundary(
            [&]
        {
            CheckAndClearOutPointer(object);

            auto newCanvasDevice = CanvasDevice::CreateNew(
                false); // Do not force software renderer

            ThrowIfFailed(newCanvasDevice.CopyTo(object));
        });
    }

    IFACEMETHODIMP CanvasDeviceFactory::GetSharedDevice(
        ICanvasDevice** device)
    {
        return GetSharedDeviceWithForceSoftwareRenderer(FALSE, device);
    }

    IFACEMETHODIMP CanvasDeviceFactory::GetSharedDeviceWithForceSoftwareRenderer(
        boolean forceSoftwareRenderer,
        ICanvasDevice** device)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckAndClearOutPointer(device);

                auto sharedDevice = SharedDeviceState::GetInstance()->GetSharedDevice(!!forceSoftwareRenderer);

                ThrowIfFailed(sharedDevice.CopyTo(device));
            });
    }

    IFACEMETHODIMP CanvasDeviceFactory::put_DebugLevel(CanvasDebugLevel debugLevel)
    {
        return ExceptionBoundary(
            [&]
            {
                SharedDeviceState::GetInstance()->SetDebugLevel(debugLevel);
            });
    }

    IFACEMETHODIMP CanvasDeviceFactory::get_DebugLevel(CanvasDebugLevel* debugLevel)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckInPointer(debugLevel);

                *debugLevel = SharedDeviceState::GetInstance()->GetDebugLevel();
            });
    }


    //
    // ICanvasFactoryNative.
    //
    // This is not really related to CanvasDevice: we just attach it to the factory
    // interface to give Microsoft.Graphics.Canvas.native.h a convenient way to access it.
    //

    IFACEMETHODIMP CanvasDeviceFactory::GetOrCreate(ICanvasDevice* device, IUnknown* resource, float dpi, IInspectable** wrapper)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckInPointer(resource);
                CheckAndClearOutPointer(wrapper);

                auto result = ResourceManager::GetOrCreate(device, resource, dpi);

                ThrowIfFailed(result.CopyTo(wrapper));
            });
    }


    //
    // CanvasDevice
    //

    CanvasDevice::CanvasDevice(
        ID2D1Device1* d2dDevice,
        IDXGIDevice3* dxgiDevice,
        bool forceSoftwareRenderer)
        : ResourceWrapper(d2dDevice)
        , m_forceSoftwareRenderer(forceSoftwareRenderer)
        , m_dxgiDevice(dxgiDevice)
        , m_sharedState(SharedDeviceState::GetInstance())
        , m_deviceContextPool(d2dDevice)
    {
        if (!dxgiDevice)
        {
            auto dxgiDeviceFromD2DDevice = m_sharedState->GetAdapter()->GetDxgiDevice(d2dDevice);
            m_dxgiDevice = dxgiDevice = dxgiDeviceFromD2DDevice.Get();
        }

        InitializePrimaryOutput(dxgiDevice);
    }

    ComPtr<CanvasDevice> CanvasDevice::CreateNew(
        bool forceSoftwareRenderer)
    {
        auto sharedState = SharedDeviceState::GetInstance();

        auto debugLevel = sharedState->GetDebugLevel();
        bool useDebugD3DDevice = debugLevel != CanvasDebugLevel::None;

        auto d2dFactory = sharedState->GetAdapter()->CreateD2DFactory(debugLevel);

        auto dxgiDevice = As<IDXGIDevice3>(MakeD3D11Device(sharedState->GetAdapter(), forceSoftwareRenderer, useDebugD3DDevice));

        ComPtr<ID2D1Device1> d2dDevice;
        ThrowIfFailed(d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice));

        auto device = Make<CanvasDevice>(d2dDevice.Get(), dxgiDevice.Get(), forceSoftwareRenderer);
        CheckMakeResult(device);

        return device;
    }

    ComPtr<CanvasDevice> CanvasDevice::CreateNew(
        IDirect3DDevice* direct3DDevice)
    {
        CheckInPointer(direct3DDevice);

        auto sharedState = SharedDeviceState::GetInstance();

        auto d2dFactory = sharedState->GetAdapter()->CreateD2DFactory(sharedState->GetDebugLevel());

        ComPtr<IDXGIDevice3> dxgiDevice;
        ThrowIfFailed(As<IDirect3DDxgiInterfaceAccess>(direct3DDevice)->GetInterface(IID_PPV_ARGS(&dxgiDevice)));

        ComPtr<ID2D1Device1> d2dDevice;
        ThrowIfFailed(d2dFactory->CreateDevice(dxgiDevice.Get(), &d2dDevice));

        auto device = Make<CanvasDevice>(d2dDevice.Get(), dxgiDevice.Get());
        CheckMakeResult(device);

        return device;
    }

    ComPtr<ID3D11Device> CanvasDevice::MakeD3D11Device(
        CanvasDeviceAdapter* adapter,
        bool forceSoftwareRenderer,
        bool useDebugD3DDevice)
    {
        ComPtr<ID3D11Device> d3dDevice;

        if (adapter->TryCreateD3DDevice(forceSoftwareRenderer, useDebugD3DDevice, &d3dDevice))
        {
            return d3dDevice;
        }

        if (!forceSoftwareRenderer)
        {
            // try again using the software renderer
            if (adapter->TryCreateD3DDevice(true, useDebugD3DDevice, &d3dDevice))
            {
                return d3dDevice;
            }
        }

        // If we end up here then we failed to create a d3d device
        ThrowHR(E_FAIL);
    }

    ComPtr<ID2D1Factory2> CanvasDevice::GetD2DFactory()
    {
        ComPtr<ID2D1Device1> d2dDevice = GetD2DDevice();

        ComPtr<ID2D1Factory> d2dBaseFactory;
        d2dDevice->GetFactory(&d2dBaseFactory);

        ComPtr<ID2D1Factory2> d2dDerivedFactory;
        d2dBaseFactory.As(&d2dDerivedFactory);

        return d2dDerivedFactory;
    }

    IFACEMETHODIMP CanvasDevice::get_ForceSoftwareRenderer(boolean *value)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckInPointer(value);
                GetResource();  // this ensures that Close() hasn't been called
                *value = m_forceSoftwareRenderer;
                return S_OK;
            });
    }


    IFACEMETHODIMP CanvasDevice::get_MaximumBitmapSizeInPixels(int32_t* value)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckInPointer(value);
                
                auto deviceContext = GetResourceCreationDeviceContext();
                UINT32 maximumBitmapSize = deviceContext->GetMaximumBitmapSize();

                assert(maximumBitmapSize <= INT_MAX);

                *value = static_cast<int32_t>(maximumBitmapSize);
            });
    }

    IFACEMETHODIMP CanvasDevice::add_DeviceLost(
        DeviceLostHandlerType* value, 
        EventRegistrationToken* token)
    {        
        return ExceptionBoundary(
            [&]
            {
                GetResource();  // this ensures that Close() hasn't been called
                CheckInPointer(value);
                CheckInPointer(token);

                ThrowIfFailed(m_deviceLostEventList.Add(value, token));
            });
    }

    IFACEMETHODIMP CanvasDevice::remove_DeviceLost(
        EventRegistrationToken token)
    {
        return ExceptionBoundary(
            [&]
            {
                //
                // This does not check if this object was closed, so to 
                // allow shutdown paths to freely remove events.
                //
                ThrowIfFailed(m_deviceLostEventList.Remove(token));
            });
    }

    IFACEMETHODIMP CanvasDevice::IsDeviceLost(
        int hresult,
        boolean* value)
    {        
        return ExceptionBoundary(
            [&]
            {
                GetResource();  // this ensures that Close() hasn't been called

                CheckInPointer(value);

                if (DeviceLostException::IsDeviceLostHResult(hresult))
                {
                    *value = GetDeviceRemovedErrorCode() != S_OK;
                }
                else
                {
                    *value = false;
                }
            });
    }

    IFACEMETHODIMP CanvasDevice::RaiseDeviceLost()
    {        
        return ExceptionBoundary(
            [&]
            {
                if (SUCCEEDED(GetDeviceRemovedErrorCode())) // Also ensures that Close() hasn't been called
                {
                    ThrowHR(E_INVALIDARG, Strings::DeviceExpectedToBeLost);
                }

                ThrowIfFailed(m_deviceLostEventList.InvokeAll(this, nullptr));
            });
    }

    IFACEMETHODIMP CanvasDevice::Close()
    {
        return ExceptionBoundary(
            [&]
            {
                m_deviceContextPool.Close();
                ThrowIfFailed(this->ResourceWrapper::Close()); // 'this->' is workaround for VS2013 calling with bad 'this' pointer

                m_dxgiDevice.Close();
                m_primaryOutput.Reset();
            });
    }

    ComPtr<ID2D1Device1> CanvasDevice::GetD2DDevice()
    {
        return GetResource();
    }

    ComPtr<ID2D1DeviceContext1> CanvasDevice::CreateDeviceContextForDrawingSession()
    {
        ComPtr<ID2D1DeviceContext1> dc;
        ThrowIfFailed(GetResource()->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &dc));
        return dc;
    }

    ComPtr<ID2D1SolidColorBrush> CanvasDevice::CreateSolidColorBrush(D2D1_COLOR_F const& color)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1SolidColorBrush> brush;
        ThrowIfFailed(deviceContext->CreateSolidColorBrush(color, &brush));

        return brush;
    }

    ComPtr<ID2D1Bitmap1> CanvasDevice::CreateBitmapFromWicResource(
        IWICBitmapSource* wicBitmapSource,
        float dpi,
        CanvasAlphaMode alpha)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1();
        bitmapProperties.pixelFormat.alphaMode = ToD2DAlphaMode(alpha);
        bitmapProperties.dpiX = bitmapProperties.dpiY = dpi;

        ComPtr<ID2D1Bitmap1> bitmap;
        ThrowIfFailed(deviceContext->CreateBitmapFromWicBitmap(wicBitmapSource, &bitmapProperties, &bitmap));

        return bitmap;
    }

    ComPtr<ID2D1Bitmap1> CanvasDevice::CreateBitmapFromBytes(
        uint8_t* bytes,
        uint32_t pitch,
        int32_t widthInPixels,
        int32_t heightInPixels,
        float dpi,
        DirectXPixelFormat format,
        CanvasAlphaMode alphaMode)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1();
        bitmapProperties.pixelFormat.alphaMode = ToD2DAlphaMode(alphaMode);
        bitmapProperties.pixelFormat.format = static_cast<DXGI_FORMAT>(format);
        bitmapProperties.dpiX = dpi;
        bitmapProperties.dpiY = dpi;

        auto size = D2D1::SizeU(widthInPixels, heightInPixels);

        ComPtr<ID2D1Bitmap1> d2dBitmap;
        ThrowIfFailed(deviceContext->CreateBitmap(
            size,
            bytes,
            pitch,
            &bitmapProperties,
            &d2dBitmap));

        return d2dBitmap;
    }

    ComPtr<ID2D1Bitmap1> CanvasDevice::CreateBitmapFromSurface(
        IDirect3DSurface* surface,
        float dpi,
        CanvasAlphaMode alphaMode)
    {        
        auto deviceContext = GetResourceCreationDeviceContext();

        auto dxgiSurface = GetDXGIInterface<IDXGISurface2>(surface);

        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1();
        bitmapProperties.pixelFormat.alphaMode = ToD2DAlphaMode(alphaMode);
        bitmapProperties.dpiX = dpi;
        bitmapProperties.dpiY = dpi;

        // D2D requires bitmap flags that match the surface format, if a
        // D2D1_BITMAP_PROPERTIES1 is specified.
        //
        ComPtr<ID3D11Texture2D> parentResource = GetTexture2DForDXGISurface(dxgiSurface);

        ComPtr<IDXGIResource1> dxgiResource;
        ThrowIfFailed(parentResource.As(&dxgiResource));

        DXGI_USAGE dxgiUsage;
        ThrowIfFailed(dxgiResource->GetUsage(&dxgiUsage));

        D3D11_TEXTURE2D_DESC texture2DDesc;
        parentResource->GetDesc(&texture2DDesc);

        if (texture2DDesc.BindFlags & D3D11_BIND_RENDER_TARGET && !(dxgiUsage & DXGI_USAGE_READ_ONLY))
        {
            bitmapProperties.bitmapOptions |= D2D1_BITMAP_OPTIONS_TARGET;
        }

        if (!(texture2DDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
        {
            bitmapProperties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
        }

        if (texture2DDesc.Usage & D3D11_USAGE_STAGING && texture2DDesc.CPUAccessFlags & D3D11_CPU_ACCESS_READ)
        {
            bitmapProperties.bitmapOptions |= D2D1_BITMAP_OPTIONS_CPU_READ;
        }
        
        ComPtr<ID2D1Bitmap1> d2dBitmap;
        ThrowIfFailed(deviceContext->CreateBitmapFromDxgiSurface(
            dxgiSurface.Get(),
            &bitmapProperties,
            &d2dBitmap));

        return d2dBitmap;
    }

    ComPtr<ID2D1Bitmap1> CanvasDevice::CreateRenderTargetBitmap(
        float width,
        float height,
        float dpi,
        DirectXPixelFormat format,
        CanvasAlphaMode alpha)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1Bitmap1> bitmap;
        D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1();
        bitmapProperties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;
        bitmapProperties.dpiX = dpi;
        bitmapProperties.dpiY = dpi;
        bitmapProperties.pixelFormat.format = static_cast<DXGI_FORMAT>(format);
        bitmapProperties.pixelFormat.alphaMode = ToD2DAlphaMode(alpha);

        auto pixelWidth = static_cast<uint32_t>(SizeDipsToPixels(width, dpi));
        auto pixelHeight = static_cast<uint32_t>(SizeDipsToPixels(height, dpi));

        ThrowIfFailed(deviceContext->CreateBitmap(
            D2D1_SIZE_U{ pixelWidth, pixelHeight },
            nullptr, // data 
            0,  // data pitch
            &bitmapProperties, 
            &bitmap));

        return bitmap;
    }
    
    IFACEMETHODIMP CanvasDevice::get_Device(ICanvasDevice** value)
    {
        return ExceptionBoundary(
            [&]
            {
                CheckAndClearOutPointer(value);
                ComPtr<ICanvasDevice> device(this);
                *value = device.Detach();
            });
    }

    ComPtr<ID2D1BitmapBrush1> CanvasDevice::CreateBitmapBrush(ID2D1Bitmap1* bitmap)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1BitmapBrush1> bitmapBrush;
        ThrowIfFailed(deviceContext->CreateBitmapBrush(bitmap, &bitmapBrush));

        return bitmapBrush;
    }

    ComPtr<ID2D1ImageBrush> CanvasDevice::CreateImageBrush(ID2D1Image* image)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1ImageBrush> imageBrush;
        ThrowIfFailed(deviceContext->CreateImageBrush(image, D2D1::ImageBrushProperties(D2D1::RectF()), &imageBrush));

        return imageBrush;
    }

    IFACEMETHODIMP CanvasDevice::Trim()
    {
        return ExceptionBoundary(
            [&]
            {
                auto& d2dDevice = GetResource();
                auto& dxgiDevice = m_dxgiDevice.EnsureNotClosed();

                D2DResourceLock lock(d2dDevice.Get());

                dxgiDevice->Trim();
            });
    }

    IFACEMETHODIMP CanvasDevice::GetInterface(REFIID iid, void** p)
    {
        return ExceptionBoundary(
            [&]
            {
                auto& dxgiDevice = m_dxgiDevice.EnsureNotClosed();

                ThrowIfFailed(dxgiDevice.CopyTo(iid, p));
            });
    }

    ComPtr<ID2D1GradientStopCollection1> CanvasDevice::CreateGradientStopCollection(
        uint32_t gradientStopCount,
        CanvasGradientStop const* gradientStops,
        CanvasEdgeBehavior edgeBehavior,
        CanvasColorSpace preInterpolationSpace,
        CanvasColorSpace postInterpolationSpace,
        CanvasBufferPrecision bufferPrecision,
        CanvasAlphaMode alphaMode)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        std::vector<D2D1_GRADIENT_STOP> d2dGradientStops;
        d2dGradientStops.resize(gradientStopCount);
        for (uint32_t i = 0; i < gradientStopCount; ++i)
        {
            d2dGradientStops[i].color = ToD2DColor(gradientStops[i].Color);
            d2dGradientStops[i].position = gradientStops[i].Position;
        }

        ComPtr<ID2D1GradientStopCollection1> gradientStopCollection;
        ThrowIfFailed(deviceContext->CreateGradientStopCollection(
            &d2dGradientStops[0],
            gradientStopCount,
            static_cast<D2D1_COLOR_SPACE>(preInterpolationSpace),
            static_cast<D2D1_COLOR_SPACE>(postInterpolationSpace),
            ToD2DBufferPrecision(bufferPrecision),
            static_cast<D2D1_EXTEND_MODE>(edgeBehavior),
            ToD2DColorInterpolation(alphaMode),
            gradientStopCollection.GetAddressOf()));

        return gradientStopCollection;
    }

    ComPtr<ID2D1LinearGradientBrush> CanvasDevice::CreateLinearGradientBrush(
        ID2D1GradientStopCollection1* stopCollection)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES linearGradientBrushProperties = D2D1::LinearGradientBrushProperties(
            D2D1::Point2F(),
            D2D1::Point2F());

        ComPtr<ID2D1LinearGradientBrush> linearGradientBrush;
        ThrowIfFailed(deviceContext->CreateLinearGradientBrush(
            linearGradientBrushProperties, 
            stopCollection,
            linearGradientBrush.GetAddressOf()));

        return linearGradientBrush;

    }

    ComPtr<ID2D1RadialGradientBrush> CanvasDevice::CreateRadialGradientBrush(
        ID2D1GradientStopCollection1* stopCollection)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialGradientBrushProperties = D2D1::RadialGradientBrushProperties(
            D2D1::Point2F(),
            D2D1::Point2F(), 
            0, 
            0);

        ComPtr<ID2D1RadialGradientBrush> radialGradientBrush;
        ThrowIfFailed(deviceContext->CreateRadialGradientBrush(
            radialGradientBrushProperties,
            stopCollection,
            radialGradientBrush.GetAddressOf()));

        return radialGradientBrush;
    }


    template<typename FN>
    ComPtr<IDXGISwapChain1> CanvasDevice::CreateSwapChain(
        int32_t widthInPixels,
        int32_t heightInPixels,
        DirectXPixelFormat format,
        int32_t bufferCount,
        CanvasAlphaMode alphaMode,
        FN&& createFn)
    {
        auto& d2dDevice = GetResource();
        auto& dxgiDevice = m_dxgiDevice.EnsureNotClosed();

        D2DResourceLock lock(d2dDevice.Get());

        ComPtr<IDXGIAdapter2> dxgiAdapter;
        ThrowIfFailed(dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter)));

        ComPtr<IDXGIFactory2> dxgiFactory;
        ThrowIfFailed(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory)));

        ThrowIfNegative(widthInPixels);
        ThrowIfNegative(heightInPixels);
        ThrowIfNegative(bufferCount);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { };
        swapChainDesc.Width = static_cast<UINT>(widthInPixels);
        swapChainDesc.Height = static_cast<UINT>(heightInPixels);
        swapChainDesc.Format = static_cast<DXGI_FORMAT>(format);
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.SampleDesc.Quality = 0;
        swapChainDesc.BufferCount = static_cast<UINT>(bufferCount);
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        swapChainDesc.AlphaMode = ToDxgiAlphaMode(alphaMode);

        ComPtr<IDXGISwapChain1> swapChain;
        ThrowIfFailed(createFn(dxgiFactory.Get(), dxgiDevice.Get(), &swapChainDesc, &swapChain));

        return swapChain;
    }


    ComPtr<IDXGISwapChain1> CanvasDevice::CreateSwapChainForComposition(
        int32_t widthInPixels,
        int32_t heightInPixels,
        DirectXPixelFormat format,
        int32_t bufferCount,
        CanvasAlphaMode alphaMode)
    {
        return CreateSwapChain(widthInPixels, heightInPixels, format, bufferCount, alphaMode,
            [] (IDXGIFactory2* factory, IDXGIDevice3* device, DXGI_SWAP_CHAIN_DESC1* desc, IDXGISwapChain1** swapChain)
            {
                return factory->CreateSwapChainForComposition(
                    device, 
                    desc, 
                    nullptr,  // restrictToOutput
                    swapChain);
            });
    }

    ComPtr<IDXGISwapChain1> CanvasDevice::CreateSwapChainForCoreWindow(
        ICoreWindow* coreWindow,
        int32_t widthInPixels,
        int32_t heightInPixels,
        DirectXPixelFormat format,
        int32_t bufferCount,
        CanvasAlphaMode alphaMode)
    {
        return CreateSwapChain(widthInPixels, heightInPixels, format, bufferCount, alphaMode,
            [coreWindow] (IDXGIFactory2* factory, IDXGIDevice3* device, DXGI_SWAP_CHAIN_DESC1* desc, IDXGISwapChain1** swapChain)
            {
                return factory->CreateSwapChainForCoreWindow(
                    device,
                    coreWindow,
                    desc,
                    nullptr,    // restrictToOutput
                    swapChain);
            });
    }


    ComPtr<ID2D1CommandList> CanvasDevice::CreateCommandList()
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1CommandList> cl;
        ThrowIfFailed(deviceContext->CreateCommandList(&cl));

        return cl;
    }

    ComPtr<ID2D1RectangleGeometry> CanvasDevice::CreateRectangleGeometry(D2D1_RECT_F const& rectangle)
    {
        auto factory = GetD2DFactory();

        ComPtr<ID2D1RectangleGeometry> rectangleGeometry;
        ThrowIfFailed(factory->CreateRectangleGeometry(rectangle, &rectangleGeometry));

        return rectangleGeometry;
    }

    ComPtr<ID2D1EllipseGeometry> CanvasDevice::CreateEllipseGeometry(D2D1_ELLIPSE const& ellipse)
    {
        auto factory = GetD2DFactory();

        ComPtr<ID2D1EllipseGeometry> ellipseGeometry;
        ThrowIfFailed(factory->CreateEllipseGeometry(ellipse, &ellipseGeometry));

        return ellipseGeometry;
    }

    ComPtr<ID2D1RoundedRectangleGeometry> CanvasDevice::CreateRoundedRectangleGeometry(D2D1_ROUNDED_RECT const& roundedRect)
    {
        auto factory = GetD2DFactory();

        ComPtr<ID2D1RoundedRectangleGeometry> roundedRectangleGeometry;
        ThrowIfFailed(factory->CreateRoundedRectangleGeometry(roundedRect, &roundedRectangleGeometry));

        return roundedRectangleGeometry;
    }

    ComPtr<ID2D1PathGeometry1> CanvasDevice::CreatePathGeometry()
    {
        auto factory = GetD2DFactory();

        ComPtr<ID2D1PathGeometry1> pathGeometry;
        ThrowIfFailed(factory->CreatePathGeometry(&pathGeometry));

        return pathGeometry;
    }

    ComPtr<ID2D1GeometryGroup> CanvasDevice::CreateGeometryGroup(D2D1_FILL_MODE fillMode, ID2D1Geometry** d2dGeometries, uint32_t geometryCount)
    {
        auto factory = GetD2DFactory();

        ComPtr<ID2D1GeometryGroup> geometryGroup;
        ThrowIfFailed(factory->CreateGeometryGroup(fillMode, d2dGeometries, geometryCount, &geometryGroup));

        return geometryGroup;
    }

    ComPtr<ID2D1TransformedGeometry> CanvasDevice::CreateTransformedGeometry(ID2D1Geometry* d2dGeometry, D2D1_MATRIX_3X2_F* transform)
    {
        auto factory = GetD2DFactory();

        ComPtr<ID2D1TransformedGeometry> transformedGeometry;
        ThrowIfFailed(factory->CreateTransformedGeometry(d2dGeometry, transform, &transformedGeometry));

        return transformedGeometry;
    }

    ComPtr<ID2D1GeometryRealization> CanvasDevice::CreateFilledGeometryRealization(ID2D1Geometry* geometry, float flatteningTolerance)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1GeometryRealization> geometryRealization;
        ThrowIfFailed(deviceContext->CreateFilledGeometryRealization(geometry, flatteningTolerance, &geometryRealization));

        return geometryRealization;
    }

    ComPtr<ID2D1GeometryRealization> CanvasDevice::CreateStrokedGeometryRealization(
        ID2D1Geometry* geometry,
        float strokeWidth,
        ID2D1StrokeStyle* strokeStyle,
        float flatteningTolerance)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        ComPtr<ID2D1GeometryRealization> geometryRealization;
        ThrowIfFailed(deviceContext->CreateStrokedGeometryRealization(
            geometry,
            flatteningTolerance,
            strokeWidth,
            strokeStyle,
            &geometryRealization));

        return geometryRealization;
    }

    ComPtr<ID2D1PrintControl> CanvasDevice::CreatePrintControl(
        IPrintDocumentPackageTarget* target,
        float dpi)
    {
        ComPtr<IWICImagingFactory> wicFactory;
        ThrowIfFailed(
            CoCreateInstance(
            CLSID_WICImagingFactory,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFactory)));

        auto properties = D2D1_PRINT_CONTROL_PROPERTIES{
            D2D1_PRINT_FONT_SUBSET_MODE_DEFAULT,
            dpi,
            D2D1_COLOR_SPACE_SRGB
        };

        auto& d2dDevice = GetResource();

        ComPtr<ID2D1PrintControl> printControl;
        ThrowIfFailed(d2dDevice->CreatePrintControl(
            wicFactory.Get(),
            target,
            properties,
            &printControl));

        return printControl;
    }

    DeviceContextLease CanvasDevice::GetResourceCreationDeviceContext()
    {
        return m_deviceContextPool.TakeLease();
    }

    void CanvasDevice::InitializePrimaryOutput(IDXGIDevice3* dxgiDevice)
    {
        D2DResourceLock lock(GetResource().Get());

        //
        // Creating a CanvasDevice using forceSoftwareRenderer==true
        // creates a 'render-only' WARP device, which cannot be used to 
        // enumerate outputs.
        //
        // This is distinct from basic display, where EnumOutputs still works normally.
        //
        // But rather than check if hardware acceleration is on or off
        // before calling EnumOutputs, we try it and just let it fail
        // if it's not supported. Reason being, interop.
        //
        ComPtr<IDXGIAdapter> adapter;
        ThrowIfFailed(dxgiDevice->GetAdapter(&adapter));

        //
        // This holds onto the primary output, which is the recommendation
        // for synchronizing with vertical blank.
        //
        // If the primary display changes to a different one, or if
        // the monitor is no longer is plugged in, WaitForVBlank will
        // be emulated and the emulated behavior has shown to be reliable.
        //
        // In the uncommon scenario where no display is plugged in at
        // all, an emulated primary will still be enumerated here.
        //
        HRESULT enumOutputsHr = adapter->EnumOutputs(0, &m_primaryOutput);
        if (FAILED(enumOutputsHr) && enumOutputsHr != DXGI_ERROR_NOT_FOUND)
        {
            ThrowHR(enumOutputsHr);
        }
    }

    ComPtr<IDXGIOutput> CanvasDevice::GetPrimaryDisplayOutput()
    {
        return m_primaryOutput;
    }

#if WINVER > _WIN32_WINNT_WINBLUE
    ComPtr<ID2D1GradientMesh> CanvasDevice::CreateGradientMesh(
        D2D1_GRADIENT_MESH_PATCH const* patches,
        uint32_t patchCount)
    {
        auto deviceContext = GetResourceCreationDeviceContext();

        auto deviceContext2 = As<ID2D1DeviceContext2>(deviceContext.Get());

        ComPtr<ID2D1GradientMesh> gradientMesh;

        ThrowIfFailed(deviceContext2->CreateGradientMesh(patches, patchCount, &gradientMesh));

        return gradientMesh;
    }
#endif

    HRESULT CanvasDevice::GetDeviceRemovedErrorCode()
    {
        auto& dxgiDevice = m_dxgiDevice.EnsureNotClosed();

        auto d3dDevice = As<ID3D11Device>(dxgiDevice);

        return d3dDevice->GetDeviceRemovedReason();
    }

    ActivatableClassWithFactory(CanvasDevice, CanvasDeviceFactory);
}}}}
