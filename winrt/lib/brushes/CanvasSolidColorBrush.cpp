// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#include "pch.h"

#include "CanvasSolidColorBrush.h"

using namespace ABI::Microsoft::Graphics::Canvas::Brushes;
using namespace ABI::Microsoft::Graphics::Canvas;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::UI;

ComPtr<CanvasSolidColorBrush> CanvasSolidColorBrush::CreateNew(
    ICanvasResourceCreator* resourceCreator,
    Color color)
{
    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(resourceCreator->get_Device(&device));

    ComPtr<ICanvasDeviceInternal> canvasDeviceInternal;
    ThrowIfFailed(device.As(&canvasDeviceInternal));

    auto d2dBrush = canvasDeviceInternal->CreateSolidColorBrush(ToD2DColor(color));

    auto canvasSolidColorBrush = Make<CanvasSolidColorBrush>(
        device.Get(),
        d2dBrush.Get());
    CheckMakeResult(canvasSolidColorBrush);

    return canvasSolidColorBrush;
};


IFACEMETHODIMP CanvasSolidColorBrushFactory::Create(
    ICanvasResourceCreator* resourceCreator,
    ABI::Windows::UI::Color color,
    ICanvasSolidColorBrush** canvasSolidColorBrush)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(resourceCreator);
            CheckAndClearOutPointer(canvasSolidColorBrush);

            auto newSolidColorBrush = CanvasSolidColorBrush::CreateNew(
                resourceCreator,
                color);

            ThrowIfFailed(newSolidColorBrush.CopyTo(canvasSolidColorBrush));
        });
}


CanvasSolidColorBrush::CanvasSolidColorBrush(
    ICanvasDevice *device,
    ID2D1SolidColorBrush* brush)
    : CanvasBrush(device)
    , ResourceWrapper(brush)
{
}

IFACEMETHODIMP CanvasSolidColorBrush::get_Color(_Out_ Color *value)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(value);
            *value = ToWindowsColor(GetResource()->GetColor());
        });
}

IFACEMETHODIMP CanvasSolidColorBrush::put_Color(_In_ Color value)
{
    return ExceptionBoundary(
        [&]
        {                
            GetResource()->SetColor(ToD2DColor(value));
        });
}

IFACEMETHODIMP CanvasSolidColorBrush::Close()
{
    CanvasBrush::Close();
    return ResourceWrapper::Close();
}

ComPtr<ID2D1Brush> CanvasSolidColorBrush::GetD2DBrush(ID2D1DeviceContext*, GetBrushFlags)
{
    return GetResource();
}

ActivatableClassWithFactory(CanvasSolidColorBrush, CanvasSolidColorBrushFactory);
