// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#include "pch.h"

#include <lib/images/CanvasCommandList.h>

#include "stubs/TestEffect.h"
#include "stubs/StubD2DEffect.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(CanvasEffectUnitTest)
{
public:

    ComPtr<TestEffect> m_testEffect;
    unsigned int m_realPropertiesSize = 4;
    unsigned int m_realSourcesSize = 8;
    GUID m_blurGuid = CLSID_D2D1GaussianBlur;

    TEST_METHOD_INITIALIZE(Reset)
    {
        m_testEffect = Make<TestEffect>(m_blurGuid, m_realPropertiesSize, m_realSourcesSize, false);
    }

    TEST_METHOD_EX(CanvasEffect_Implements_Expected_Interfaces)
    {
        ASSERT_IMPLEMENTS_INTERFACE(m_testEffect, IGraphicsEffect);
        ASSERT_IMPLEMENTS_INTERFACE(m_testEffect, IGraphicsEffectSource);
        ASSERT_IMPLEMENTS_INTERFACE(m_testEffect, ABI::Windows::Foundation::IClosable);
        ASSERT_IMPLEMENTS_INTERFACE(m_testEffect, ICanvasImage);
        ASSERT_IMPLEMENTS_INTERFACE(m_testEffect, ICanvasImageInternal);
        ASSERT_IMPLEMENTS_INTERFACE(m_testEffect, IGaussianBlurEffect);
    }

    TEST_METHOD_EX(CanvasEffect_Properties)
    {
        unsigned int propertiesSize;
        ThrowIfFailed(m_testEffect->GetPropertyCount(&propertiesSize));
        Assert::AreEqual(m_realPropertiesSize, propertiesSize);
        for (unsigned int i = 0; i < m_realPropertiesSize; ++i)
        {
            ComPtr<IPropertyValue> propertyValue;
            ThrowIfFailed(m_testEffect->GetProperty(i, &propertyValue));
            Assert::IsNull(propertyValue.Get());
        }

        bool isSetPropertyCalled = false;
        m_testEffect->MockSetProperty =
            [&]
            {
                Assert::IsFalse(isSetPropertyCalled);
                isSetPropertyCalled = true;
            };

        bool isGetPropertyCalled = false;
        m_testEffect->MockGetProperty =
            [&]
            {
                Assert::IsFalse(isGetPropertyCalled);
                isGetPropertyCalled = true;
            };

        float value = 5.0f;
        ThrowIfFailed(m_testEffect->put_BlurAmount(value));
        ThrowIfFailed(m_testEffect->get_BlurAmount(&value));

        Assert::IsTrue(isSetPropertyCalled);
        Assert::IsTrue(isGetPropertyCalled);
    }

    TEST_METHOD_EX(CanvasEffect_Closed)
    {
        ABI::Windows::Foundation::Rect bounds;
        Numerics::Matrix3x2 matrix = { 0 };

        auto canvasEffect = Make<TestEffect>(m_blurGuid, m_realPropertiesSize, m_realSourcesSize, false);
        auto drawingSession = CreateStubDrawingSession();

        Assert::AreEqual(S_OK, canvasEffect->Close());

        Assert::AreEqual(RO_E_CLOSED, canvasEffect->GetBounds(drawingSession.Get(), &bounds));
        Assert::AreEqual(RO_E_CLOSED, canvasEffect->GetBoundsWithTransform(drawingSession.Get(), matrix, &bounds));

        ComPtr<ID2D1Effect> resource;
        Assert::AreEqual(RO_E_CLOSED, canvasEffect->GetNativeResource(nullptr, 0, IID_PPV_ARGS(&resource)));
    }

    TEST_METHOD_EX(CanvasEffect_Sources)
    {
        unsigned int sourceSize;
        ThrowIfFailed(m_testEffect->GetSourceCount(&sourceSize));
        Assert::AreEqual(m_realSourcesSize, sourceSize);
        for (unsigned int i = 0; i < m_realSourcesSize; ++i)
        {
            ComPtr<IGraphicsEffectSource> source;
            ThrowIfFailed(m_testEffect->GetSource(i, &source));
            Assert::IsNull(source.Get());
        }

        bool isSetSourceCalled = false;
        m_testEffect->MockSetSource =
            [&]
            {
                Assert::IsFalse(isSetSourceCalled);
                isSetSourceCalled = true;
            };

        bool isGetSourceCalled = false;
        m_testEffect->MockGetSource =
            [&]
            {
                Assert::IsFalse(isGetSourceCalled);
                isGetSourceCalled = true;
            };

        // set same effect as source
        ThrowIfFailed(m_testEffect->put_Source(m_testEffect.Get()));
        ComPtr<IGraphicsEffectSource> source;
        ThrowIfFailed(m_testEffect->get_Source(&source));

        Assert::IsTrue(isSetSourceCalled);
        Assert::IsTrue(isGetSourceCalled);

        Assert::AreEqual(As<IUnknown>(source).Get(), As<IUnknown>(m_testEffect).Get());

        // set null source
        isSetSourceCalled = isGetSourceCalled = false;

        ThrowIfFailed(m_testEffect->put_Source(nullptr));
        ThrowIfFailed(m_testEffect->get_Source(&source));

        Assert::IsTrue(isSetSourceCalled);
        Assert::IsTrue(isGetSourceCalled);

        Assert::IsNull(source.Get());
    }

    struct Fixture
    {
        ComPtr<StubD2DDevice> m_stubDevice;
        ComPtr<StubD2DDeviceContextWithGetFactory> m_deviceContext;
        ComPtr<CanvasDrawingSession> m_drawingSession;
        ComPtr<StubCanvasDevice> m_canvasDevice;
        float m_dpi;

        Fixture()
        {
            m_stubDevice = Make<StubD2DDevice>();
            m_deviceContext = MakeDeviceContext();
            m_canvasDevice = Make<StubCanvasDevice>(m_stubDevice);
            m_drawingSession = CanvasDrawingSession::CreateNew(m_deviceContext.Get(), std::make_shared<StubCanvasDrawingSessionAdapter>(), m_canvasDevice.Get());
            m_dpi = DEFAULT_DPI;
        }

        ComPtr<StubD2DDeviceContextWithGetFactory> MakeDeviceContext()
        {
            auto deviceContext = Make<StubD2DDeviceContextWithGetFactory>();

            deviceContext->GetDeviceMethod.AllowAnyCallAlwaysCopyValueToParam(m_stubDevice);
            deviceContext->GetPrimitiveBlendMethod.AllowAnyCall();

            deviceContext->GetDpiMethod.AllowAnyCall(
                [&](float* dpiX, float* dpiY)
                {
                    *dpiX = m_dpi;
                    *dpiY = m_dpi;
                });

            deviceContext->GetTargetMethod.AllowAnyCall(
                [] (ID2D1Image** target)
                {
                    *target = nullptr;
                });

            return deviceContext;
        }
    };

    TEST_METHOD_EX(CanvasEffect_Rerealize)
    {
        // Use a test effect with one source and one property.
        auto testEffect = Make<TestEffect>(m_blurGuid, 1, 1, false);

        // This test realizes m_testEffect twice, each time with different 
        // devices. Verifies that the D2D effect is re-created, and 
        // its sources and properties are set.
        for (int pass = 0; pass < 2; pass++)
        {
            Fixture f;

            // Set a source and non-default value.
            auto stubBitmap = CreateStubCanvasBitmap(DEFAULT_DPI, f.m_canvasDevice.Get());

            ThrowIfFailed(testEffect->put_Source(As<IGraphicsEffectSource>(stubBitmap).Get()));
            ThrowIfFailed(testEffect->put_BlurAmount(99));

            f.m_deviceContext->DrawImageMethod.AllowAnyCall();

            bool setInputCalled = false;
            bool setInputCountCalled = false;
            bool setValueCalled = false;

            f.m_deviceContext->CreateEffectMethod.SetExpectedCalls(1,
                [&](IID const&, ID2D1Effect** effect)
                {
                    ComPtr<MockD2DEffect> mockEffect = Make<MockD2DEffect>();
                    mockEffect.CopyTo(effect);

                    mockEffect->MockSetInput =
                        [&](UINT32, ID2D1Image*)
                        {
                            Assert::IsFalse(setInputCalled);
                            setInputCalled = true;
                        };

                    mockEffect->MockGetInput =
                        [&](UINT32, ID2D1Image** input)
                        {
                            ThrowIfFailed(stubBitmap->GetD2DImage(nullptr, nullptr, (GetImageFlags)0, 0, nullptr).CopyTo(input));
                        };

                    mockEffect->MockGetInputCount =
                        []()
                        {
                            return 1;
                        };

                    mockEffect->MockSetInputCount =
                        [&](UINT32)
                        {
                            Assert::IsFalse(setInputCountCalled);
                            setInputCountCalled = true;
                            return S_OK;
                        };

                    mockEffect->MockSetValue =
                        [&](UINT32, D2D1_PROPERTY_TYPE, CONST BYTE*, UINT32)
                        {
                            Assert::IsFalse(setValueCalled);
                            setValueCalled = true;
                            return S_OK;
                        };

                    mockEffect->MockGetType =
                        [](UINT32)
                        {
                            return D2D1_PROPERTY_TYPE_FLOAT;
                        };

                    mockEffect->MockGetValue =
                        [](UINT32 index, D2D1_PROPERTY_TYPE type, BYTE *data, UINT32 dataSize)
                        {
                            ZeroMemory(data, dataSize);
                            return S_OK;
                        };

                    return S_OK;
                });

            f.m_drawingSession = nullptr;
            f.m_drawingSession = CanvasDrawingSession::CreateNew(f.m_deviceContext.Get(), std::make_shared<StubCanvasDrawingSessionAdapter>(), f.m_canvasDevice.Get());

            ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

            Assert::IsTrue(setInputCalled);
            Assert::IsTrue(setInputCountCalled);
            Assert::IsTrue(setValueCalled);

            // Drawing on a second device context that shares the same device should NOT rerealize the effect.
            auto deviceContext2 = f.MakeDeviceContext();
        
            deviceContext2->DrawImageMethod.AllowAnyCall();
            deviceContext2->CreateEffectMethod.SetExpectedCalls(0);

            auto drawingSession2 = CanvasDrawingSession::CreateNew(deviceContext2.Get(), std::make_shared<StubCanvasDrawingSessionAdapter>(), f.m_canvasDevice.Get());

            ThrowIfFailed(drawingSession2->DrawImageAtOrigin(testEffect.Get()));
        }
    };

    class InvalidEffectSourceType : public RuntimeClass<IGraphicsEffectSource>
    {
        InspectableClass(L"InvalidEffectSourceType", BaseTrust);
    };

    TEST_METHOD_EX(CanvasEffect_WrongSourceType)
    {
        Fixture f;

        f.m_deviceContext->CreateEffectMethod.AllowAnyCall(
            [&](IID const&, ID2D1Effect** effect)
            {
                return Make<StubD2DEffect>(m_blurGuid).CopyTo(effect);
            });

        auto testEffect = Make<TestEffect>(m_blurGuid, 0, 1, false);

        // Validate drawing with a null source.
        Assert::AreEqual(E_INVALIDARG, f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));
        
        // Validate drawing with a source that is not the right type.
        auto invalidSource = Make<InvalidEffectSourceType>();

        testEffect->SetSource(0, invalidSource.Get());

        Assert::AreEqual(E_NOINTERFACE, f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        ValidateStoredErrorState(E_NOINTERFACE, L"Effect source #0 is an unsupported type. To draw an effect using Win2D, all its sources must be Win2D ICanvasImage objects.");
    }

    TEST_METHOD_EX(CanvasEffect_CyclicGraph)
    {
        Fixture f;

        f.m_deviceContext->CreateEffectMethod.AllowAnyCall(
            [&](IID const&, ID2D1Effect** effect)
            {
                return Make<StubD2DEffect>(m_blurGuid).CopyTo(effect);
            });

        auto testEffect = Make<TestEffect>(m_blurGuid, 0, 1, false);

        testEffect->put_Source(testEffect.Get());

        Assert::AreEqual(D2DERR_CYCLIC_GRAPH, f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        // Break the cycle so we don't leak memory.
        testEffect->put_Source(nullptr);
    }

    TEST_METHOD_EX(CanvasEffect_GetBounds_NullArg)
    {
        ABI::Windows::Foundation::Rect bounds;
        Numerics::Matrix3x2 matrix = { 0 };

        auto canvasEffect = Make<TestEffect>(m_blurGuid, m_realPropertiesSize, m_realSourcesSize, false);
        auto drawingSession = CreateStubDrawingSession();

        Assert::AreEqual(E_INVALIDARG, canvasEffect->GetBounds(nullptr, &bounds));
        Assert::AreEqual(E_INVALIDARG, canvasEffect->GetBounds(drawingSession.Get(), nullptr));
        Assert::AreEqual(E_INVALIDARG, canvasEffect->GetBoundsWithTransform(nullptr, matrix, &bounds));
        Assert::AreEqual(E_INVALIDARG, canvasEffect->GetBoundsWithTransform(drawingSession.Get(), matrix, nullptr));
    }

    TEST_METHOD_EX(CanvasEffect_RealizationRecursion)
    {
        Fixture f;

        auto stubBitmap = CreateStubCanvasBitmap(DEFAULT_DPI, f.m_canvasDevice.Get());

        std::vector<ComPtr<MockD2DEffectThatCountsCalls>> mockEffects;

        auto createCountingEffect =
            [&](IID const&, ID2D1Effect** effect)
            {
                auto mockEffect = Make<MockD2DEffectThatCountsCalls>();

                mockEffect->MockGetType =
                    [](UINT32)
                    {
                        return D2D1_PROPERTY_TYPE_FLOAT;
                    };

                mockEffect->MockGetValue =
                    [](UINT32 index, D2D1_PROPERTY_TYPE type, BYTE *data, UINT32 dataSize)
                    {
                        ZeroMemory(data, dataSize);
                        return S_OK;
                    };

                mockEffects.push_back(mockEffect);

                return mockEffect.CopyTo(effect);
            };

        std::vector<ComPtr<TestEffect>> testEffects;

        f.m_deviceContext->CreateEffectMethod.AllowAnyCall(createCountingEffect);        
        f.m_deviceContext->DrawImageMethod.AllowAnyCall();

        f.m_canvasDevice->GetResourceCreationDeviceContextMethod.AllowAnyCall(
            [&]
            {
                return DeviceContextLease(As<ID2D1DeviceContext1>(f.m_deviceContext));
            });

        // Create three effects, connected as each other's sources.
        for (int i = 0; i < 3; i++)
        {
            testEffects.push_back(Make<TestEffect>(m_blurGuid, 1, 1, false));
        }

        testEffects[0]->put_Source(testEffects[1].Get());
        testEffects[1]->put_Source(testEffects[2].Get());
        testEffects[2]->put_Source(stubBitmap.Get());

        testEffects[0]->put_BlurAmount(0);
        testEffects[1]->put_BlurAmount(0);
        testEffects[2]->put_BlurAmount(0);

        // Drawing the first time should set properties and sources on all three effects.
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 1, 1, 1 }, { 1, 1, 1 });

        // Drawing again with no configuration changes should not re-set any state through to D2D.
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 1, 1, 1 }, { 1, 1, 1 });

        // Draw after changing a source of the root effect.
        testEffects[0]->put_Source(testEffects[1].Get());
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 2, 1, 1 }, { 1, 1, 1 });

        // Draw after changing a source of the second level effect.
        testEffects[1]->put_Source(testEffects[2].Get());
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 1 }, { 1, 1, 1 });

        // Draw after changing a source of the third level effect.
        testEffects[2]->put_Source(stubBitmap.Get());
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 2 }, { 1, 1, 1 });

        // Draw after changing a property of the root effect.
        testEffects[0]->put_BlurAmount(1);
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 2 }, { 2, 1, 1 });

        // Draw after changing a property of the second level effect.
        testEffects[1]->put_BlurAmount(1);
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 2 }, { 2, 2, 1 });

        // Draw after changing a property of the third level effect.
        testEffects[2]->put_BlurAmount(1);
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[0].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 2 }, { 2, 2, 2 });

        // Draw starting at the second level of the graph should not re-set anything.
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[1].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 2 }, { 2, 2, 2 });

        // Draw starting at the third level of the graph should not re-set anything.
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffects[2].Get()));
        CheckCallCount(mockEffects, 3, { 2, 2, 2 }, { 2, 2, 2 });
    }

    static void CheckCallCount(std::vector<ComPtr<MockD2DEffectThatCountsCalls>> const& mockEffects,
                               size_t expectedEffectCount,
                               std::initializer_list<int> const& expectedSetInputCalls,
                               std::initializer_list<int> const& expectedSetValueCalls)
    {
        Assert::AreEqual(expectedEffectCount, mockEffects.size());
        Assert::AreEqual(expectedEffectCount, expectedSetInputCalls.size());
        Assert::AreEqual(expectedEffectCount, expectedSetValueCalls.size());

        auto effect = mockEffects.begin();
        auto expectedSetInputs = expectedSetInputCalls.begin();
        auto expectedSetValues = expectedSetValueCalls.begin();

        for (size_t i = 0; i < expectedEffectCount; i++)
        {
            wchar_t input_msg[256], value_msg[256];

            swprintf_s(input_msg, L"effect #%Id inputs", i);
            swprintf_s(value_msg, L"effect #%Id values", i);

            Assert::AreEqual(*expectedSetInputs, (*effect)->m_setInputCalls, input_msg);
            Assert::AreEqual(*expectedSetValues, (*effect)->m_setValueCalls, value_msg);

            ++effect;
            ++expectedSetInputs;
            ++expectedSetValues;
        }
    }

    TEST_METHOD_EX(CanvasEffect_DpiCompensation)
    {
        Fixture f;

        f.m_deviceContext->DrawImageMethod.AllowAnyCall();

        std::vector<ComPtr<MockD2DEffectThatCountsCalls>> mockEffects;

        f.m_deviceContext->CreateEffectMethod.AllowAnyCall(
            [&](IID const& effectId, ID2D1Effect** effect)
            {
                mockEffects.push_back(Make<MockD2DEffectThatCountsCalls>(effectId));
                return mockEffects.back().CopyTo(effect);
            });

        f.m_canvasDevice->GetResourceCreationDeviceContextMethod.AllowAnyCall(
            [&]
            {
                return DeviceContextLease(As<ID2D1DeviceContext1>(f.m_deviceContext));
            });

        // First we draw a default DPI bitmap onto a default DPI context. This should not insert any DPI compensation.
        auto testBitmap = CreateStubCanvasBitmap(DEFAULT_DPI, f.m_canvasDevice.Get());
        auto testEffect = Make<TestEffect>(m_blurGuid, 0, 1, true);

        ThrowIfFailed(testEffect->put_Source(testBitmap.Get()));
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        Assert::AreEqual<size_t>(1, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, testBitmap.Get(), f.m_deviceContext.Get());

        // Now draw a high DPI bitmap. This should insert a DPI compensation effect.
        const float highDpi = 144;
        auto highDpiBitmap = CreateStubCanvasBitmap(highDpi, f.m_canvasDevice.Get());

        ThrowIfFailed(testEffect->put_Source(highDpiBitmap.Get()));
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        Assert::AreEqual<size_t>(2, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, mockEffects[1].Get());
        CheckEffectTypeAndInput(mockEffects[1].Get(), CLSID_D2D1DpiCompensation, highDpiBitmap.Get(), f.m_deviceContext.Get(), highDpi);

        // Drawing a different bitmap should reuse the existing DPI compensation effect.
        const float highDpi2 = 192;
        auto highDpiBitmap2 = CreateStubCanvasBitmap(highDpi2, f.m_canvasDevice.Get());

        ThrowIfFailed(testEffect->put_Source(highDpiBitmap2.Get()));
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        Assert::AreEqual<size_t>(2, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, mockEffects[1].Get());
        CheckEffectTypeAndInput(mockEffects[1].Get(), CLSID_D2D1DpiCompensation, highDpiBitmap2.Get(), f.m_deviceContext.Get(), highDpi2);

        // Drawing a high DPI bitmap that matches a high DPI device context should remove the DPI compensation effect.
        f.m_dpi = highDpi;

        ThrowIfFailed(testEffect->put_Source(highDpiBitmap.Get()));
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        Assert::AreEqual<size_t>(2, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, highDpiBitmap.Get(), f.m_deviceContext.Get());

        // Drawing a high DPI bitmap that doesn't match a different high DPI device context should insert a new DPI compensation effect.
        f.m_dpi = highDpi2;

        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        Assert::AreEqual<size_t>(3, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, mockEffects[2].Get());
        CheckEffectTypeAndInput(mockEffects[2].Get(), CLSID_D2D1DpiCompensation, highDpiBitmap.Get(), f.m_deviceContext.Get(), highDpi);

        // If we insert our own DPI compensation effect in the chain, Win2D should not automatically add a new one.
        auto dpiCompensationEffect = Make<TestEffect>(CLSID_D2D1DpiCompensation, 0, 1, true);

        ThrowIfFailed(testEffect->put_Source(dpiCompensationEffect.Get()));
        ThrowIfFailed(dpiCompensationEffect->put_Source(highDpiBitmap.Get()));
        ThrowIfFailed(f.m_drawingSession->DrawImageAtOrigin(testEffect.Get()));

        Assert::AreEqual<size_t>(4, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, mockEffects[3].Get());
        CheckEffectTypeAndInput(mockEffects[3].Get(), CLSID_D2D1DpiCompensation, highDpiBitmap.Get(), f.m_deviceContext.Get());
        Assert::IsTrue(IsSameInstance(mockEffects[3].Get(), As<ICanvasImageInternal>(dpiCompensationEffect)->GetD2DImage(f.m_canvasDevice.Get(), f.m_deviceContext.Get()).Get()));
    }

    struct CommandListFixture
    {
        ComPtr<StubCanvasDevice> CanvasDevice;
        ComPtr<MockD2DDeviceContext> DeviceContext;
        std::vector<ComPtr<MockD2DEffectThatCountsCalls>> MockEffects;
        ComPtr<ID2D1Image> CurrentTarget;
        ComPtr<CanvasCommandListFactory> CommandListFactory;

        CommandListFixture()
            : CanvasDevice(Make<StubCanvasDevice>())
            , DeviceContext(Make<MockD2DDeviceContext>())
            , CommandListFactory(Make<CanvasCommandListFactory>())
        {
            DeviceContext->CreateEffectMethod.AllowAnyCall(
                [=](IID const& effectId, ID2D1Effect** effect)
                {
                    MockEffects.push_back(Make<MockD2DEffectThatCountsCalls>(effectId));
                    return MockEffects.back().CopyTo(effect);
                });
            
            DeviceContext->BeginDrawMethod.AllowAnyCall();
            DeviceContext->EndDrawMethod.AllowAnyCall();
            DeviceContext->DrawImageMethod.AllowAnyCall();
            DeviceContext->GetPrimitiveBlendMethod.AllowAnyCall();
            DeviceContext->SetTextAntialiasModeMethod.AllowAnyCall();

            DeviceContext->SetTargetMethod.SetExpectedCalls(1,
                [&] (ID2D1Image* newTarget)
                {
                    CurrentTarget = newTarget;
                });
            
            DeviceContext->GetTargetMethod.SetExpectedCalls(1,
                [&] (ID2D1Image** value)
                {
                    CurrentTarget.CopyTo(value);
                });

            DeviceContext->GetDeviceMethod.AllowAnyCallAlwaysCopyValueToParam(CanvasDevice->GetD2DDevice());

            // CanvasEffect must not query the DPI when the device context is being
            // used with a command list.
            DeviceContext->GetDpiMethod.SetExpectedCalls(0);

            CanvasDevice->CreateCommandListMethod.AllowAnyCall(
                [] 
                { 
                    auto d2dCl = Make<MockD2DCommandList>();
                    d2dCl->CloseMethod.AllowAnyCall();
                    return d2dCl;
                });

            CanvasDevice->CreateDeviceContextForDrawingSessionMethod.SetExpectedCalls(1, [=] { return DeviceContext; });            
        }

        ComPtr<ICanvasCommandList> CreateCommandList()
        {
            ComPtr<ICanvasCommandList> cl;
            ThrowIfFailed(CommandListFactory->Create(CanvasDevice.Get(), &cl));
            return cl;
        }

        void DrawEffectToCommandList(ComPtr<TestEffect> effect)
        {
            auto cl = CreateCommandList();

            ComPtr<ICanvasDrawingSession> drawingSession;
            ThrowIfFailed(cl->CreateDrawingSession(&drawingSession));

            ThrowIfFailed(drawingSession->DrawImageAtOrigin(effect.Get()));
        }        
    };

    TEST_METHOD_EX(CanvasEffect_When_BitmapSource_DrawnIntoCommandList_DpiCompensationAlwaysAdded)
    {
        for (auto dpi : { DEFAULT_DPI / 2, DEFAULT_DPI, DEFAULT_DPI * 2 })
        {
            Check_BitmapSource_DrawnIntoCommandList_DpiCompensationAdded(dpi);
        }
    }

    void Check_BitmapSource_DrawnIntoCommandList_DpiCompensationAdded(float dpi)
    {
        CommandListFixture f;

        auto testBitmap = CreateStubCanvasBitmap(dpi, f.CanvasDevice.Get());
        auto testEffect = Make<TestEffect>(m_blurGuid, 0, 1, true);
        ThrowIfFailed(testEffect->put_Source(testBitmap.Get()));

        f.DrawEffectToCommandList(testEffect);

        Assert::AreEqual<size_t>(2, f.MockEffects.size());
        CheckEffectTypeAndInput(f.MockEffects[0].Get(), m_blurGuid, f.MockEffects[1].Get());
        CheckEffectTypeAndInput(f.MockEffects[1].Get(), CLSID_D2D1DpiCompensation, testBitmap.Get(), f.DeviceContext.Get(), dpi);
    }

    TEST_METHOD_EX(CanvasEffect_When_DpiIndependentEffect_DrawingIntoCommandList_NoDpiCompensationIsAdded)
    {
        CommandListFixture f;

        auto commandListUsedAsEffectSource = f.CreateCommandList();
        auto testEffect = Make<TestEffect>(m_blurGuid, 0, 1, true);
        ThrowIfFailed(testEffect->put_Source(As<IGraphicsEffectSource>(commandListUsedAsEffectSource).Get()));

        f.DrawEffectToCommandList(testEffect);

        Assert::AreEqual<size_t>(1, f.MockEffects.size());
        Assert::IsTrue(!!IsEqualGUID(m_blurGuid, f.MockEffects[0]->m_effectId));
    }

    TEST_METHOD_EX(CanvasEffect_EffectAsImageBrushSource_DpiCompensation)
    {
        SwitchableTestBrushFixture f;

        // Feed a bitmap into an effect into the image brush.
        auto testBitmap = CreateStubCanvasBitmap(DEFAULT_DPI, f.m_canvasDevice.Get());
        auto testEffect = Make<TestEffect>(m_blurGuid, 0, 1, true);
        
        std::vector<ComPtr<MockD2DEffectThatCountsCalls>> mockEffects;

        f.m_d2dDeviceContext->CreateEffectMethod.AllowAnyCall(
            [&](IID const& effectId, ID2D1Effect** effect)
        {
            mockEffects.push_back(Make<MockD2DEffectThatCountsCalls>(effectId));
            return mockEffects.back().CopyTo(effect);
        });

        ThrowIfFailed(testEffect->put_Source(testBitmap.Get()));

        ThrowIfFailed(f.m_canvasImageBrush->put_Image(testEffect.Get()));

        auto sourceRectangle = Make<Nullable<Rect>>(Rect{ 0, 0, 1, 1 });
        ThrowIfFailed(f.m_canvasImageBrush->put_SourceRectangle(sourceRectangle.Get()));

        // Create a drawing session.
        auto drawingSession = CanvasDrawingSession::CreateNew(f.m_d2dDeviceContext.Get(), std::make_shared<StubCanvasDrawingSessionAdapter>(), f.m_canvasDevice.Get());

        f.m_d2dDeviceContext->FillRectangleMethod.AllowAnyCall();
        f.m_d2dDeviceContext->GetDeviceMethod.AllowAnyCallAlwaysCopyValueToParam(f.m_canvasDevice->GetD2DDevice());
        f.m_d2dDeviceContext->GetTargetMethod.AllowAnyCallAlwaysCopyValueToParam<ID2D1Image>(nullptr);

        float currentDpi = DEFAULT_DPI;

        f.m_d2dDeviceContext->GetDpiMethod.AllowAnyCall(
            [&](float* dpiX, float* dpiY)
        {
            *dpiX = currentDpi;
            *dpiY = currentDpi;
        });

        // Drawing at default DPI should realize the effect graph, but not insert any DPI compensation.
        ThrowIfFailed(drawingSession->FillRectangleAtCoordsWithBrush(0, 0, 0, 0, f.m_canvasImageBrush.Get()));

        Assert::AreEqual<size_t>(1, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, testBitmap.Get(), f.m_d2dDeviceContext.Get());

        // Drawing with the image brush at a different DPI should insert a DPI compensation effect.
        currentDpi *= 2;

        ThrowIfFailed(drawingSession->FillRectangleAtCoordsWithBrush(0, 0, 0, 0, f.m_canvasImageBrush.Get()));

        Assert::AreEqual<size_t>(2, mockEffects.size());
        CheckEffectTypeAndInput(mockEffects[0].Get(), m_blurGuid, mockEffects[1].Get());
        CheckEffectTypeAndInput(mockEffects[1].Get(), CLSID_D2D1DpiCompensation, testBitmap.Get(), f.m_d2dDeviceContext.Get(), DEFAULT_DPI);
    }

    // DImage defines separate (but identical) enum types for different effects.
    // Effects codegen tool collapses this duplication in the WinRT projection.
    // Let's validate that the native enums really are the same!

    static_assert(D2D1_3DTRANSFORM_INTERPOLATION_MODE_NEAREST_NEIGHBOR == (int)CanvasImageInterpolation::NearestNeighbor, "enums should match");
    static_assert(D2D1_3DTRANSFORM_INTERPOLATION_MODE_LINEAR == (int)CanvasImageInterpolation::Linear, "enums should match");
    static_assert(D2D1_3DTRANSFORM_INTERPOLATION_MODE_CUBIC == (int)CanvasImageInterpolation::Cubic, "enums should match");
    static_assert(D2D1_3DTRANSFORM_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR == (int)CanvasImageInterpolation::MultiSampleLinear, "enums should match");
    static_assert(D2D1_3DTRANSFORM_INTERPOLATION_MODE_ANISOTROPIC == (int)CanvasImageInterpolation::Anisotropic, "enums should match");

    static_assert(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_NEAREST_NEIGHBOR == (int)CanvasImageInterpolation::NearestNeighbor, "enums should match");
    static_assert(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_LINEAR == (int)CanvasImageInterpolation::Linear, "enums should match");
    static_assert(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_CUBIC == (int)CanvasImageInterpolation::Cubic, "enums should match");
    static_assert(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR == (int)CanvasImageInterpolation::MultiSampleLinear, "enums should match");
    static_assert(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_ANISOTROPIC == (int)CanvasImageInterpolation::Anisotropic, "enums should match");
    static_assert(D2D1_2DAFFINETRANSFORM_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC == (int)CanvasImageInterpolation::HighQualityCubic, "enums should match");

    static_assert(D2D1_CONVOLVEMATRIX_SCALE_MODE_NEAREST_NEIGHBOR == (int)CanvasImageInterpolation::NearestNeighbor, "enums should match");
    static_assert(D2D1_CONVOLVEMATRIX_SCALE_MODE_LINEAR == (int)CanvasImageInterpolation::Linear, "enums should match");
    static_assert(D2D1_CONVOLVEMATRIX_SCALE_MODE_CUBIC == (int)CanvasImageInterpolation::Cubic, "enums should match");
    static_assert(D2D1_CONVOLVEMATRIX_SCALE_MODE_MULTI_SAMPLE_LINEAR == (int)CanvasImageInterpolation::MultiSampleLinear, "enums should match");
    static_assert(D2D1_CONVOLVEMATRIX_SCALE_MODE_ANISOTROPIC == (int)CanvasImageInterpolation::Anisotropic, "enums should match");
    static_assert(D2D1_CONVOLVEMATRIX_SCALE_MODE_HIGH_QUALITY_CUBIC == (int)CanvasImageInterpolation::HighQualityCubic, "enums should match");

    static_assert(D2D1_BORDER_EDGE_MODE_CLAMP == (int)CanvasEdgeBehavior::Clamp, "enums should match");
    static_assert(D2D1_BORDER_EDGE_MODE_WRAP == (int)CanvasEdgeBehavior::Wrap, "enums should match");
    static_assert(D2D1_BORDER_EDGE_MODE_MIRROR == (int)CanvasEdgeBehavior::Mirror, "enums should match");
};
