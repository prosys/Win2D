// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#include "pch.h"

#include "CanvasGeometry.h"
#include "CanvasPathBuilder.h"
#include "GeometrySink.h"
#include "TessellationSink.h"

using namespace ABI::Microsoft::Graphics::Canvas::Geometry;
using namespace ABI::Microsoft::Graphics::Canvas;

static const Matrix3x2 Identity3x2 = { 1, 0, 0, 1, 0, 0 };

IFACEMETHODIMP CanvasGeometryFactory::CreateRectangle(
    ICanvasResourceCreator* resourceCreator,
    Rect rect,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(resourceCreator);
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(resourceCreator, rect);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometryFactory::CreateRectangleAtCoords(
    ICanvasResourceCreator* resourceCreator,
    float x,
    float y,
    float w,
    float h,
    ICanvasGeometry** geometry)
{
    return CreateRectangle(resourceCreator, Rect{ x, y, w, h }, geometry);
}

IFACEMETHODIMP CanvasGeometryFactory::CreateRoundedRectangle(
    ICanvasResourceCreator* resourceCreator,
    Rect rect,
    float xRadius,
    float yRadius,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(resourceCreator);
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(resourceCreator, rect, xRadius, yRadius);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometryFactory::CreateRoundedRectangleAtCoords(
    ICanvasResourceCreator* resourceCreator,
    float x,
    float y,
    float w,
    float h,
    float xRadius,
    float yRadius,
    ICanvasGeometry** geometry)
{
    return CreateRoundedRectangle(resourceCreator, Rect{ x, y, w, h }, xRadius, yRadius, geometry);
}

IFACEMETHODIMP CanvasGeometryFactory::CreateEllipse(
    ICanvasResourceCreator* resourceCreator,
    Numerics::Vector2 center,
    float xRadius,
    float yRadius,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(resourceCreator);
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(resourceCreator, center, xRadius, yRadius);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometryFactory::CreateEllipseAtCoords(
    ICanvasResourceCreator* resourceCreator,
    float centerX,
    float centerY,
    float xRadius,
    float yRadius,
    ICanvasGeometry** geometry)
{
    return CreateEllipse(resourceCreator, Vector2{ centerX, centerY }, xRadius, yRadius, geometry);
}

IFACEMETHODIMP CanvasGeometryFactory::CreateCircle(
    ICanvasResourceCreator* resourceCreator,
    Numerics::Vector2 center,
    float radius,
    ICanvasGeometry** geometry)
{
    return CreateEllipse(resourceCreator, center, radius, radius, geometry);
}

IFACEMETHODIMP CanvasGeometryFactory::CreateCircleAtCoords(
    ICanvasResourceCreator* resourceCreator,
    float centerX,
    float centerY,
    float radius,
    ICanvasGeometry** geometry)
{
    return CreateEllipse(resourceCreator, Vector2{ centerX, centerY }, radius, radius, geometry);
}

IFACEMETHODIMP CanvasGeometryFactory::CreatePath(
    ICanvasPathBuilder* pathBuilder,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(pathBuilder);
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(pathBuilder);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
    });
}

IFACEMETHODIMP CanvasGeometryFactory::CreatePolygon(
    ICanvasResourceCreator* resourceCreator,
    uint32_t pointCount,
    Numerics::Vector2* points,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(resourceCreator); 
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(resourceCreator, pointCount, points);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometryFactory::CreateGroup(
    ICanvasResourceCreator* resourceCreator,
    uint32_t geometryCount,
    ICanvasGeometry** geometryElements,
    ICanvasGeometry** geometry)
{
    return CreateGroupWithFilledRegionDetermination(
        resourceCreator,
        geometryCount,
        geometryElements,
        CanvasFilledRegionDetermination::Alternate,
        geometry);
}

IFACEMETHODIMP CanvasGeometryFactory::CreateGroupWithFilledRegionDetermination(
    ICanvasResourceCreator* resourceCreator,
    uint32_t geometryCount,
    ICanvasGeometry** geometryElements,
    CanvasFilledRegionDetermination filledRegionDetermination,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(resourceCreator); 
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(resourceCreator, geometryCount, geometryElements, filledRegionDetermination);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometryFactory::CreateText(
    ICanvasTextLayout* textLayout,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(textLayout);
            CheckAndClearOutPointer(geometry);

            auto newCanvasGeometry = CanvasGeometry::CreateNew(textLayout);

            ThrowIfFailed(newCanvasGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometryFactory::ComputeFlatteningTolerance(
    float dpi,
    float maximumZoomFactor,
    float* flatteningTolerance)
{
    return ComputeFlatteningToleranceWithTransform(dpi, maximumZoomFactor, Identity3x2, flatteningTolerance);
}

// Ideally we would just call D2D1::ComputeFlatteningTolerance here, which internally uses
// D2D1ComputeMaximumScaleFactor, but unfortunately that DLL entrypoint is not marked as
// valid for Windows Phone 8.1 apps (an oversight). Using it would make Win2D Phone apps
// fail certification, so instead we must do the calculation directly here ourselves.
static float ComputeMaximumScaleFactor(D2D1_MATRIX_3X2_F const& m)
{
    if (m._12 == 0.0f && m._21 == 0.0f)
    {
        // Simple scale matrix.
        return std::max(fabs(m._11), fabs(m._22));
    }
    else
    {
        // Solve a quadratic.
        float a = m._11 * m._11 + m._12 * m._12;
        float b = m._11 * m._21 + m._12 * m._22;
        float c = m._21 * m._21 + m._22 * m._22;

        float d = a - c;

        float r = sqrtf(d * d + b * b * 4);

        return sqrtf((a + c + r) * 0.5f);
    }
}

IFACEMETHODIMP CanvasGeometryFactory::ComputeFlatteningToleranceWithTransform(
    float dpi,
    float maximumZoomFactor,
    Matrix3x2 expectedGeometryTransform,
    float* flatteningTolerance)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(flatteningTolerance);

            float dpiScale = dpi / DEFAULT_DPI;

            D2D1_MATRIX_3X2_F dpiDependentTransform = *ReinterpretAs<D2D1_MATRIX_3X2_F*>(&expectedGeometryTransform) *
                                                      D2D1::Matrix3x2F::Scale(dpiScale, dpiScale);

            float scaleFactor = fabs(maximumZoomFactor) * ComputeMaximumScaleFactor(dpiDependentTransform);

            *flatteningTolerance = D2D1_DEFAULT_FLATTENING_TOLERANCE / scaleFactor;
        });
}

IFACEMETHODIMP CanvasGeometryFactory::get_DefaultFlatteningTolerance(float* theValue)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(theValue);

            *theValue = D2D1_DEFAULT_FLATTENING_TOLERANCE;
        });
}

CanvasGeometry::CanvasGeometry(
    ICanvasDevice* canvasDevice,
    ID2D1Geometry* d2dGeometry)
    : ResourceWrapper(d2dGeometry)
    , m_canvasDevice(canvasDevice)
{
}

IFACEMETHODIMP CanvasGeometry::Close()
{
    m_canvasDevice.Close();
    return ResourceWrapper::Close();
}

IFACEMETHODIMP CanvasGeometry::get_Device(ICanvasDevice** device)
{
    return ExceptionBoundary(
        [&]
        {
            CheckAndClearOutPointer(device);

            ThrowIfFailed(m_canvasDevice.EnsureNotClosed().CopyTo(device));
        });
}


IFACEMETHODIMP CanvasGeometry::CombineWith(
    ICanvasGeometry* otherGeometry,
    Matrix3x2 otherGeometryTransform,
    CanvasGeometryCombine combine,
    ICanvasGeometry** geometry)
{
    return CombineWithUsingFlatteningTolerance(
        otherGeometry,
        otherGeometryTransform,
        combine,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        geometry);
}

IFACEMETHODIMP CanvasGeometry::CombineWithUsingFlatteningTolerance(
    ICanvasGeometry* otherGeometry,
    Matrix3x2 otherGeometryTransform,
    CanvasGeometryCombine combine,
    float flatteningTolerance,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(otherGeometry);
            CheckAndClearOutPointer(geometry);

            auto& resource = GetResource();

            auto temporaryPathBuilder = Make<CanvasPathBuilder>(m_canvasDevice.EnsureNotClosed().Get());
            CheckMakeResult(temporaryPathBuilder);
            auto targetPathBuilderInternal = As<ICanvasPathBuilderInternal>(temporaryPathBuilder);

            ThrowIfFailed(resource->CombineWithGeometry(
                GetWrappedResource<ID2D1Geometry>(otherGeometry).Get(),
                static_cast<D2D1_COMBINE_MODE>(combine),
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&otherGeometryTransform),
                flatteningTolerance,
                targetPathBuilderInternal->GetGeometrySink().Get())); 

            auto newGeometry = CanvasGeometry::CreateNew(temporaryPathBuilder.Get());
            ThrowIfFailed(newGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometry::Stroke(
    float strokeWidth,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            return StrokeImpl(strokeWidth, nullptr, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, geometry);
        });
}

IFACEMETHODIMP CanvasGeometry::StrokeWithStrokeStyle(
    float strokeWidth,
    ICanvasStrokeStyle *strokeStyle,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(strokeStyle);
            return StrokeImpl(strokeWidth, strokeStyle, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, geometry);
        });
}

IFACEMETHODIMP CanvasGeometry::StrokeWithAllOptions(
    float strokeWidth,
    ICanvasStrokeStyle *strokeStyle,
    Matrix3x2 transform,
    float flatteningTolerance,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(strokeStyle);
            return StrokeImpl(strokeWidth, strokeStyle, &transform, flatteningTolerance, geometry);
        });
}

void CanvasGeometry::StrokeImpl(
    float strokeWidth,
    ICanvasStrokeStyle* strokeStyle,
    Matrix3x2* transform,
    float flatteningTolerance,
    ICanvasGeometry** geometry)
{
    CheckAndClearOutPointer(geometry);

    auto& resource = GetResource();

    auto temporaryPathBuilder = Make<CanvasPathBuilder>(m_canvasDevice.EnsureNotClosed().Get());
    CheckMakeResult(temporaryPathBuilder);
    auto targetPathBuilderInternal = As<ICanvasPathBuilderInternal>(temporaryPathBuilder);

    ThrowIfFailed(resource->Widen(
        strokeWidth,
        MaybeGetStrokeStyleResource(resource.Get(), strokeStyle).Get(),
        ReinterpretAs<D2D1_MATRIX_3X2_F*>(transform),
        flatteningTolerance,
        targetPathBuilderInternal->GetGeometrySink().Get()));

    auto newGeometry = CanvasGeometry::CreateNew(temporaryPathBuilder.Get());
    ThrowIfFailed(newGeometry.CopyTo(geometry));
}

IFACEMETHODIMP CanvasGeometry::Outline(
    ICanvasGeometry** geometry)
{
    return OutlineWithTransformAndFlatteningTolerance(
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        geometry);
}

IFACEMETHODIMP CanvasGeometry::OutlineWithTransformAndFlatteningTolerance(
    Matrix3x2 transform,
    float flatteningTolerance,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckAndClearOutPointer(geometry);

            auto& resource = GetResource();

            auto temporaryPathBuilder = Make<CanvasPathBuilder>(m_canvasDevice.EnsureNotClosed().Get());
            CheckMakeResult(temporaryPathBuilder);
            auto targetPathBuilderInternal = As<ICanvasPathBuilderInternal>(temporaryPathBuilder);

            ThrowIfFailed(resource->Outline(
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
                flatteningTolerance,
                targetPathBuilderInternal->GetGeometrySink().Get())); 

            auto newGeometry = CanvasGeometry::CreateNew(temporaryPathBuilder.Get());
            ThrowIfFailed(newGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometry::Simplify(
    CanvasGeometrySimplification simplification,
    ICanvasGeometry** geometry)
{
    return SimplifyWithTransformAndFlatteningTolerance(
        simplification,
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        geometry);
}

IFACEMETHODIMP CanvasGeometry::SimplifyWithTransformAndFlatteningTolerance(
    CanvasGeometrySimplification simplification,
    Matrix3x2 transform,
    float flatteningTolerance,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckAndClearOutPointer(geometry);

            auto& resource = GetResource();

            auto temporaryPathBuilder = Make<CanvasPathBuilder>(m_canvasDevice.EnsureNotClosed().Get());
            CheckMakeResult(temporaryPathBuilder);
            auto targetPathBuilderInternal = As<ICanvasPathBuilderInternal>(temporaryPathBuilder);

            ThrowIfFailed(resource->Simplify(
                static_cast<D2D1_GEOMETRY_SIMPLIFICATION_OPTION>(simplification),
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
                flatteningTolerance,
                targetPathBuilderInternal->GetGeometrySink().Get())); 

            auto newGeometry = CanvasGeometry::CreateNew(temporaryPathBuilder.Get());
            ThrowIfFailed(newGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometry::Transform(
    Numerics::Matrix3x2 transform,
    ICanvasGeometry** geometry)
{
    return ExceptionBoundary(
        [&]
        {
            CheckAndClearOutPointer(geometry);

            auto& resource = GetResource();

            auto& device = m_canvasDevice.EnsureNotClosed();

            auto deviceInternal = As<ICanvasDeviceInternal>(device.Get());

            auto d2dGeometry = deviceInternal->CreateTransformedGeometry(
                resource.Get(),
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform));

            auto newGeometry = Make<CanvasGeometry>(device.Get(), static_cast<ID2D1Geometry*>(d2dGeometry.Get()));
            CheckMakeResult(newGeometry);

            ThrowIfFailed(newGeometry.CopyTo(geometry));
        });
}

IFACEMETHODIMP CanvasGeometry::CompareWith(
    ICanvasGeometry* otherGeometry,
    CanvasGeometryRelation* relation)
{
    return CompareWithUsingTransformAndFlatteningTolerance(
        otherGeometry,
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        relation);
}

IFACEMETHODIMP CanvasGeometry::CompareWithUsingTransformAndFlatteningTolerance(
    ICanvasGeometry* otherGeometry,
    Matrix3x2 otherGeometryTransform,
    float flatteningTolerance,
    CanvasGeometryRelation* relation)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(otherGeometry);
            CheckInPointer(relation);

            auto& resource = GetResource();

            D2D1_GEOMETRY_RELATION d2dRelation;

            ThrowIfFailed(resource->CompareWithGeometry(
                GetWrappedResource<ID2D1Geometry>(otherGeometry).Get(),
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&otherGeometryTransform),
                flatteningTolerance,
                &d2dRelation));

            switch (d2dRelation)
            {
                case D2D1_GEOMETRY_RELATION_DISJOINT: *relation = CanvasGeometryRelation::Disjoint; break;
                case D2D1_GEOMETRY_RELATION_IS_CONTAINED: *relation = CanvasGeometryRelation::Contained; break;
                case D2D1_GEOMETRY_RELATION_CONTAINS: *relation = CanvasGeometryRelation::Contains; break;
                case D2D1_GEOMETRY_RELATION_OVERLAP: *relation = CanvasGeometryRelation::Overlap; break;
                case D2D1_GEOMETRY_RELATION_UNKNOWN:
                default:
                    assert(false); // Unexpected value returned from D2D.
                    ThrowHR(E_UNEXPECTED);
            }
        });
}

IFACEMETHODIMP CanvasGeometry::ComputeArea(
    float* area)
{
    return ComputeAreaWithTransformAndFlatteningTolerance(
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        area);
}

IFACEMETHODIMP CanvasGeometry::ComputeAreaWithTransformAndFlatteningTolerance(
    Matrix3x2 transform,
    float flatteningTolerance,
    float* area)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(area);

            auto& resource = GetResource();

            FLOAT d2dArea;

            ThrowIfFailed(resource->ComputeArea(
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
                flatteningTolerance, 
                &d2dArea));

            *area = d2dArea;
        });
}

IFACEMETHODIMP CanvasGeometry::ComputePathLength(
    float* length)
{
    return ComputePathLengthWithTransformAndFlatteningTolerance(
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        length);
}

IFACEMETHODIMP CanvasGeometry::ComputePathLengthWithTransformAndFlatteningTolerance(
    Matrix3x2 transform,
    float flatteningTolerance,
    float* length)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(length);

            auto& resource = GetResource();

            FLOAT d2dLength;

            ThrowIfFailed(resource->ComputeLength(
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
                flatteningTolerance, 
                &d2dLength));

            *length = d2dLength;
        });
}

IFACEMETHODIMP CanvasGeometry::ComputePointOnPath(
    float distance,
    Vector2* point)
{
    return ExceptionBoundary(
        [&]
        {
            ComputePointOnPathImpl(distance, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, nullptr, point);
        });
}

IFACEMETHODIMP CanvasGeometry::ComputePointOnPathWithTangent(
    float distance,
    Vector2* tangent,
    Vector2* point)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(tangent);

            ComputePointOnPathImpl(distance, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, tangent, point);
        });
}


IFACEMETHODIMP CanvasGeometry::ComputePointOnPathWithTransformAndFlatteningToleranceAndTangent(
    float distance,
    Matrix3x2 transform,
    float flatteningTolerance,
    Vector2* tangent,
    Vector2* point)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(tangent);

            ComputePointOnPathImpl(distance, &transform, flatteningTolerance, tangent, point);
        });
}


void CanvasGeometry::ComputePointOnPathImpl(
    float distance,
    Matrix3x2* transform,
    float flatteningTolerance,
    Vector2* tangent,
    Vector2* point)
{
    CheckInPointer(point);

    auto& resource = GetResource();

    D2D1_POINT_2F d2dPoint;
    D2D1_POINT_2F d2dUnitTangentVector;

    ThrowIfFailed(resource->ComputePointAtLength(
        distance,
        ReinterpretAs<D2D1_MATRIX_3X2_F*>(transform),
        flatteningTolerance,
        &d2dPoint,
        &d2dUnitTangentVector));

    *point = FromD2DPoint(d2dPoint);

    if (tangent)
    {
        *tangent = FromD2DPoint(d2dUnitTangentVector);
    }
}

IFACEMETHODIMP CanvasGeometry::FillContainsPoint(
    Vector2 point,
    boolean* containsPoint)
{
    return FillContainsPointWithTransformAndFlatteningTolerance(
        point,
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        containsPoint);
}

IFACEMETHODIMP CanvasGeometry::FillContainsPointWithTransformAndFlatteningTolerance(
    Vector2 point,
    Matrix3x2 transform,
    float flatteningTolerance,
    boolean* containsPoint)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(containsPoint);

            auto& resource = GetResource();

            BOOL d2dContainsPoint;

            ThrowIfFailed(resource->FillContainsPoint(
                ToD2DPoint(point),
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
                flatteningTolerance,
                &d2dContainsPoint));

            *containsPoint = !!d2dContainsPoint;
        });
}

IFACEMETHODIMP CanvasGeometry::ComputeBounds(
    Rect* bounds)
{
    return ComputeBoundsWithTransform(Identity3x2, bounds);
}

IFACEMETHODIMP CanvasGeometry::ComputeBoundsWithTransform(
    Matrix3x2 transform,
    Rect* bounds)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(bounds);

            auto& resource = GetResource();

            D2D1_RECT_F d2dBounds;

            ThrowIfFailed(resource->GetBounds(
                ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
                &d2dBounds));

            *bounds = FromD2DRect(d2dBounds);
        });
}

IFACEMETHODIMP CanvasGeometry::ComputeStrokeBounds(
    float strokeWidth,
    Rect* bounds)
{
    return ExceptionBoundary(
        [&]
        {
            ComputeStrokeBoundsImpl(strokeWidth, nullptr, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, bounds);
        });
}

IFACEMETHODIMP CanvasGeometry::ComputeStrokeBoundsWithStrokeStyle(
    float strokeWidth,
    ICanvasStrokeStyle *strokeStyle,
    Rect* bounds)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(strokeStyle);
            ComputeStrokeBoundsImpl(strokeWidth, strokeStyle, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, bounds);
        });
}

IFACEMETHODIMP CanvasGeometry::ComputeStrokeBoundsWithAllOptions(
    float strokeWidth,
    ICanvasStrokeStyle *strokeStyle,
    Matrix3x2 transform,
    float flatteningTolerance,
    Rect* bounds)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(strokeStyle);
            ComputeStrokeBoundsImpl(strokeWidth, strokeStyle, &transform, flatteningTolerance, bounds);
        });
}

void CanvasGeometry::ComputeStrokeBoundsImpl(
    float strokeWidth,
    ICanvasStrokeStyle* strokeStyle,
    Matrix3x2* transform,
    float flatteningTolerance,
    Rect* bounds)
{
    CheckInPointer(bounds);

    auto& resource = GetResource();

    D2D1_RECT_F d2dBounds;

    ThrowIfFailed(resource->GetWidenedBounds(
        strokeWidth,
        MaybeGetStrokeStyleResource(resource.Get(), strokeStyle).Get(),
        ReinterpretAs<D2D1_MATRIX_3X2_F*>(transform),
        flatteningTolerance,
        &d2dBounds));

    *bounds = FromD2DRect(d2dBounds);
}

IFACEMETHODIMP CanvasGeometry::StrokeContainsPoint(
    Vector2 point,
    float strokeWidth,
    boolean* containsPoint)
{
    return ExceptionBoundary(
        [&]
    {
        StrokeContainsPointImpl(point, strokeWidth, nullptr, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, containsPoint);
    });
}

IFACEMETHODIMP CanvasGeometry::StrokeContainsPointWithStrokeStyle(
    Vector2 point,
    float strokeWidth,
    ICanvasStrokeStyle *strokeStyle,
    boolean* containsPoint)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(strokeStyle);
            StrokeContainsPointImpl(point, strokeWidth, strokeStyle, nullptr, D2D1_DEFAULT_FLATTENING_TOLERANCE, containsPoint);
        });
}

IFACEMETHODIMP CanvasGeometry::StrokeContainsPointWithAllOptions(
    Vector2 point,
    float strokeWidth,
    ICanvasStrokeStyle *strokeStyle,
    Matrix3x2 transform,
    float flatteningTolerance,
    boolean* containsPoint)
{
    return ExceptionBoundary(
        [&]
        {
            CheckInPointer(strokeStyle);
            StrokeContainsPointImpl(point, strokeWidth, strokeStyle, &transform, flatteningTolerance, containsPoint);
        });
}

void CanvasGeometry::StrokeContainsPointImpl(
    Vector2 point,
    float strokeWidth,
    ICanvasStrokeStyle* strokeStyle,
    Matrix3x2* transform,
    float flatteningTolerance,
    boolean* containsPoint)
{
    CheckInPointer(containsPoint);

    auto& resource = GetResource();

    BOOL d2dContainsPoint;

    ThrowIfFailed(resource->StrokeContainsPoint(
        ToD2DPoint(point),
        strokeWidth,
        MaybeGetStrokeStyleResource(resource.Get(), strokeStyle).Get(),
        ReinterpretAs<D2D1_MATRIX_3X2_F*>(transform),
        flatteningTolerance,
        &d2dContainsPoint));

    *containsPoint = !!d2dContainsPoint;
}

IFACEMETHODIMP CanvasGeometry::Tessellate(
    UINT32* trianglesCount,
    CanvasTriangleVertices** triangles)
{
    return TessellateWithTransformAndFlatteningTolerance(
        Identity3x2,
        D2D1_DEFAULT_FLATTENING_TOLERANCE,
        trianglesCount,
        triangles);
}

IFACEMETHODIMP CanvasGeometry::TessellateWithTransformAndFlatteningTolerance(
    Matrix3x2 transform,
    float flatteningTolerance,
    UINT32* trianglesCount,
    CanvasTriangleVertices** triangles)
{
    return ExceptionBoundary([&]
    {
        CheckInPointer(trianglesCount);
        CheckAndClearOutPointer(triangles);

        auto& resource = GetResource();

        auto tessellationSink = Make<TessellationSink>();
        CheckMakeResult(tessellationSink);

        ThrowIfFailed(resource->Tessellate(
            ReinterpretAs<D2D1_MATRIX_3X2_F*>(&transform),
            flatteningTolerance,
            tessellationSink.Get()));

        auto outputArray = tessellationSink->GetTriangles();
        outputArray.Detach(trianglesCount, triangles);
    });
}

IFACEMETHODIMP CanvasGeometry::SendPathTo(
    ICanvasPathReceiver* streamReader)
{
    return ExceptionBoundary([&]
    {
        CheckInPointer(streamReader);

        auto& resource = GetResource();

        auto pathGeometry = MaybeAs<ID2D1PathGeometry>(resource);

        auto geometrySink = Make<GeometrySink>(streamReader);
        CheckMakeResult(geometrySink);

        if (pathGeometry)
        {
            ThrowIfFailed(pathGeometry->Stream(geometrySink.Get()));
        }
        else
        {
            ThrowIfFailed(resource->Simplify(
                D2D1_GEOMETRY_SIMPLIFICATION_OPTION_CUBICS_AND_LINES,
                nullptr,
                geometrySink.Get()));
        }
        ThrowIfFailed(geometrySink->Close());
    });
}

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasResourceCreator* resourceCreator,
    Rect rect)
{
    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(resourceCreator->get_Device(&device));

    auto deviceInternal = As<ICanvasDeviceInternal>(device);

    auto d2dGeometry = deviceInternal->CreateRectangleGeometry(ToD2DRect(rect));

    auto canvasGeometry = Make<CanvasGeometry>(
        device.Get(),
        d2dGeometry.Get());
    CheckMakeResult(canvasGeometry);

    return canvasGeometry;
}

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasResourceCreator* resourceCreator,
    Vector2 center,
    float radiusX,
    float radiusY)
{
    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(resourceCreator->get_Device(&device));

    auto deviceInternal = As<ICanvasDeviceInternal>(device);

    auto d2dGeometry = deviceInternal->CreateEllipseGeometry(D2D1::Ellipse(ToD2DPoint(center), radiusX, radiusY));

    auto canvasGeometry = Make<CanvasGeometry>(
        device.Get(),
        d2dGeometry.Get());
    CheckMakeResult(canvasGeometry);

    return canvasGeometry;
}

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasResourceCreator* resourceCreator,
    Rect rect,
    float radiusX,
    float radiusY)
{
    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(resourceCreator->get_Device(&device));

    auto deviceInternal = As<ICanvasDeviceInternal>(device);

    auto d2dGeometry = deviceInternal->CreateRoundedRectangleGeometry(D2D1::RoundedRect(ToD2DRect(rect), radiusX, radiusY));

    auto canvasGeometry = Make<CanvasGeometry>(
        device.Get(),
        d2dGeometry.Get());
    CheckMakeResult(canvasGeometry);

    return canvasGeometry;
}

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasPathBuilder* pathBuilder)
{
    auto pathBuilderInternal = As<ICanvasPathBuilderInternal>(pathBuilder);

    auto device = pathBuilderInternal->GetDevice();

    auto d2dGeometry = pathBuilderInternal->CloseAndReturnPath();

    auto canvasGeometry = Make<CanvasGeometry>(
        device.Get(),
        d2dGeometry.Get());
    CheckMakeResult(canvasGeometry);

    return canvasGeometry;
}

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasResourceCreator* resourceCreator,
    uint32_t pointCount,
    Vector2* points)
{
    if (pointCount > 0)
    {
        CheckInPointer(points);
    }

    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(resourceCreator->get_Device(&device));

    auto pathGeometry = As<ICanvasDeviceInternal>(device)->CreatePathGeometry();

    ComPtr<ID2D1GeometrySink> geometrySink;
    ThrowIfFailed(pathGeometry->Open(&geometrySink));

    if (pointCount > 0)
    {
        geometrySink->BeginFigure(ToD2DPoint(points[0]), D2D1_FIGURE_BEGIN_FILLED);

        for (uint32_t i = 1; i < pointCount; i++)
        {
            geometrySink->AddLine(ToD2DPoint(points[i]));
        }

        geometrySink->EndFigure(D2D1_FIGURE_END_CLOSED);
    }

    ThrowIfFailed(geometrySink->Close());

    auto canvasGeometry = Make<CanvasGeometry>(device.Get(), pathGeometry.Get());
    CheckMakeResult(canvasGeometry);

    return canvasGeometry;
}

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasResourceCreator* resourceCreator,
    uint32_t geometryCount,
    ICanvasGeometry** geometryElements,
    CanvasFilledRegionDetermination filledRegionDetermination)
{
    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(resourceCreator->get_Device(&device));

    auto deviceInternal = As<ICanvasDeviceInternal>(device);

    std::vector<ID2D1Geometry*> d2dGeometriesRaw;
    d2dGeometriesRaw.resize(geometryCount);

    if (geometryCount > 0)
    {
        CheckInPointer(geometryElements);
    }

    for (uint32_t i = 0; i < geometryCount; ++i)
    {
        CheckInPointer(geometryElements[i]);
        auto d2dResource = GetWrappedResource<ID2D1Geometry>(geometryElements[i]);

        //
        // This takes advantage of the fact that the elements of geometryElements
        // are alive for the duration of this method, and so are their D2D resources.
        //
        d2dGeometriesRaw[i] = d2dResource.Get();
    }

    if (geometryCount == 0)
    {
        d2dGeometriesRaw.push_back(nullptr);
    }

    auto d2dGeometry = deviceInternal->CreateGeometryGroup(
        static_cast<D2D1_FILL_MODE>(filledRegionDetermination),
        &d2dGeometriesRaw[0], 
        geometryCount);

    auto canvasGeometry = Make<CanvasGeometry>(
        device.Get(),
        d2dGeometry.Get());
    CheckMakeResult(canvasGeometry);

    return canvasGeometry;
}


class OutlineTextRenderer : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IDWriteTextRenderer1>,
    private LifespanTracker<OutlineTextRenderer>
{
    std::vector<ComPtr<ID2D1Geometry>> m_geometries;
    ComPtr<ICanvasDeviceInternal> m_device;
    ComPtr<IDWriteTextAnalyzer1> m_textAnalyzer;
public:
    OutlineTextRenderer(ComPtr<ICanvasDevice> const& device)
        : m_device(As<ICanvasDeviceInternal>(device))
    {
        auto customFontManager = CustomFontManager::GetInstance();
        auto dwriteFactory = customFontManager->GetSharedFactory();

        ComPtr<IDWriteTextAnalyzer> textAnalyzerBase;
        ThrowIfFailed(dwriteFactory->CreateTextAnalyzer(&textAnalyzerBase));
        m_textAnalyzer = As<IDWriteTextAnalyzer1>(textAnalyzerBase);
    }

    ComPtr<ID2D1GeometryGroup> CloseAndGetPath()
    {
        if (!m_geometries.empty())
        {
            std::vector<ID2D1Geometry*> rawPtrs;
            rawPtrs.resize(m_geometries.size());

            for (uint32_t i = 0; i < m_geometries.size(); ++i)
            {
                rawPtrs[i] = m_geometries[i].Get();
            }

            return m_device->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE, &rawPtrs[0], static_cast<uint32_t>(rawPtrs.size()));
        }
        else
        {
            ID2D1Geometry* unused{};
            return m_device->CreateGeometryGroup(D2D1_FILL_MODE_ALTERNATE, &unused, 0);
        }

    }

    IFACEMETHODIMP DrawGlyphRun(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        IUnknown* clientDrawingEffect)
    {
        return DrawGlyphRun(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES,
            measuringMode,
            glyphRun,
            glyphRunDescription,
            clientDrawingEffect);
    }

    IFACEMETHODIMP DrawGlyphRun(
        void*,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_MEASURING_MODE,
        DWRITE_GLYPH_RUN const* glyphRun,
        DWRITE_GLYPH_RUN_DESCRIPTION const*,
        IUnknown*)
    {
        auto d2dPathGeometry = m_device->CreatePathGeometry();

        ComPtr<ID2D1GeometrySink> d2dGeometrySink;
        ThrowIfFailed(d2dPathGeometry->Open(&d2dGeometrySink));

        ThrowIfFailed(glyphRun->fontFace->GetGlyphRunOutline(
            glyphRun->fontEmSize,
            glyphRun->glyphIndices,
            glyphRun->glyphAdvances,
            glyphRun->glyphOffsets,
            glyphRun->glyphCount,
            glyphRun->isSideways,
            glyphRun->bidiLevel % 2,
            d2dGeometrySink.Get()));

        ThrowIfFailed(d2dGeometrySink->Close());

        DWRITE_MATRIX transform;
        m_textAnalyzer->GetGlyphOrientationTransform(orientationAngle, glyphRun->isSideways, &transform);
        transform.dx = baselineOriginX;
        transform.dy = baselineOriginY;

        auto transformedGeometry = m_device->CreateTransformedGeometry(d2dPathGeometry.Get(), reinterpret_cast<D2D1_MATRIX_3X2_F*>(&transform));

        m_geometries.push_back(transformedGeometry);

        return S_OK;
    }

    D2D1_RECT_F RotateRectangle(DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle, D2D1_RECT_F const& rect)
    {
        switch (orientationAngle)
        {
            case DWRITE_GLYPH_ORIENTATION_ANGLE_90_DEGREES:
                return D2D1::RectF(-rect.top, rect.left, -rect.bottom, rect.right);
            case DWRITE_GLYPH_ORIENTATION_ANGLE_180_DEGREES:
                return D2D1::RectF(-rect.left, -rect.top, -rect.right, -rect.bottom);
            case DWRITE_GLYPH_ORIENTATION_ANGLE_270_DEGREES:
                D2D1::RectF(rect.top, -rect.left, rect.bottom, -rect.right);
            case DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES:
            default:
                return rect;
        }
    }

    D2D1_RECT_F OffsetRectangle(float offsetX, float offsetY, D2D1_RECT_F const& rect)
    {
        return D2D1::RectF(rect.left + offsetX, rect.top + offsetY, rect.right + offsetX, rect.bottom + offsetY);
    }

    IFACEMETHODIMP DrawUnderline(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_UNDERLINE const* underline,
        IUnknown* clientDrawingEffect)
    {
        DrawUnderline(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES,
            underline,
            clientDrawingEffect);
        return S_OK;
    }

    IFACEMETHODIMP DrawUnderline(
        void*,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_UNDERLINE const* underline,
        IUnknown*)
    {
        D2D1_RECT_F rect = { 0, underline->offset, underline->width, underline->offset + underline->thickness };
        rect = RotateRectangle(orientationAngle, rect);
        rect = OffsetRectangle(baselineOriginX, baselineOriginY, rect);

        auto rectangleGeometry = m_device->CreateRectangleGeometry(rect);

        m_geometries.push_back(rectangleGeometry);

        return S_OK;
    }

    IFACEMETHODIMP DrawStrikethrough(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown* clientDrawingEffect)
    {
        return DrawStrikethrough(
            clientDrawingContext,
            baselineOriginX,
            baselineOriginY,
            DWRITE_GLYPH_ORIENTATION_ANGLE_0_DEGREES,
            strikethrough,
            clientDrawingEffect);
    }

    IFACEMETHODIMP DrawStrikethrough(
        void*,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_GLYPH_ORIENTATION_ANGLE orientationAngle,
        DWRITE_STRIKETHROUGH const* strikethrough,
        IUnknown*)
    {
        D2D1_RECT_F rect = { 0, strikethrough->offset, strikethrough->width, strikethrough->offset + strikethrough->thickness };
        rect = RotateRectangle(orientationAngle, rect);
        rect = OffsetRectangle(baselineOriginX, baselineOriginY, rect);

        auto rectangleGeometry = m_device->CreateRectangleGeometry(rect);

        m_geometries.push_back(rectangleGeometry);

        return S_OK;
    }

    IFACEMETHODIMP DrawInlineObject(
        void*,
        FLOAT,
        FLOAT,
        IDWriteInlineObject*,
        BOOL,
        BOOL,
        IUnknown*)
    {
        return S_OK;
    }

    IFACEMETHODIMP DrawInlineObject(
        void*,
        FLOAT,
        FLOAT,
        DWRITE_GLYPH_ORIENTATION_ANGLE,
        IDWriteInlineObject*,
        BOOL,
        BOOL,
        IUnknown*)
    {
        //
        // TODO: 5629 Win2D does not have full exposure for inline objects yet.
        //
        return S_OK;
    }

    IFACEMETHODIMP IsPixelSnappingDisabled(
        void*,
        BOOL* isDisabled)
    {
        //
        // Outlines can't take advantage of pixel snapping because 
        // we don't know what target, positioning etc to which the
        // text will ultimately be drawn, or if it will even be
        // drawn 'intact'. 
        //
        // The conventional text rasterizing path (i.e. DrawTextLayout)
        // is what takes advantage of pixel-snapping and hinting.
        //
        *isDisabled = TRUE;
        return S_OK;
    }

    IFACEMETHODIMP GetCurrentTransform(
        void*,
        DWRITE_MATRIX* transform)
    {
        //
        // This transform is redundant with the ability to wrap the
        // output geometry in a transformed geometry, so we don't
        // expose a way of modifying it here.
        //
        CheckInPointer(transform);

        D2D1_MATRIX_3X2_F identity = D2D1::Matrix3x2F::Identity();
        *(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform)) = identity;

        return S_OK;
    }

    IFACEMETHODIMP GetPixelsPerDip(
        void*,
        FLOAT* pixelsPerDip)
    {
        //
        // Outlines are returned at a default DPI; geometries recieve
        // DPI scaling when they are drawn.
        // It is not lossy to produce outlines at a low(er) DPI and then
        // scale them up later because getting glyph outlines should 
        // never cause a flatten, and because pixel hinting doesn't apply
        // here.
        //

        const float assumedDpi = DEFAULT_DPI;
        *pixelsPerDip = assumedDpi / DEFAULT_DPI;

        return S_OK;
    }
};

ComPtr<CanvasGeometry> CanvasGeometry::CreateNew(
    ICanvasTextLayout* canvasTextLayout)
{
    auto dwriteTextLayout = GetWrappedResource<IDWriteTextLayout2>(canvasTextLayout);

    ComPtr<ICanvasDevice> device;
    ThrowIfFailed(canvasTextLayout->get_Device(&device));

    auto outlineTextRenderer = Make<OutlineTextRenderer>(device);

    ThrowIfFailed(dwriteTextLayout->Draw(nullptr, outlineTextRenderer.Get(), 0, 0));

    auto d2dGeometry = outlineTextRenderer->CloseAndGetPath();

    auto outlineGeometry = Make<CanvasGeometry>(device.Get(), d2dGeometry.Get());

    return outlineGeometry;
}

ActivatableClassWithFactory(CanvasGeometry, CanvasGeometryFactory);
