// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#pragma once

namespace ABI { namespace Microsoft { namespace Graphics { namespace Canvas { namespace Text
{

    //
    // Conversion between DWite and Canvas types.  Many of the enums are
    // blittable; this is confirmed through static_asserts before casting.
    //

#define CHECK_ENUM_MEMBER(DWRITE, CANVAS)                                           \
        static_assert(static_cast<int>(DWRITE) == static_cast<int>(CANVAS), #CANVAS " is assumed to match " #DWRITE)


    inline DWRITE_FONT_STRETCH ToFontStretch(ABI::Windows::UI::Text::FontStretch value)
    {
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_UNDEFINED, ABI::Windows::UI::Text::FontStretch_Undefined);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_ULTRA_CONDENSED, ABI::Windows::UI::Text::FontStretch_UltraCondensed);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_EXTRA_CONDENSED, ABI::Windows::UI::Text::FontStretch_ExtraCondensed);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_CONDENSED, ABI::Windows::UI::Text::FontStretch_Condensed);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_SEMI_CONDENSED, ABI::Windows::UI::Text::FontStretch_SemiCondensed);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_NORMAL, ABI::Windows::UI::Text::FontStretch_Normal);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_MEDIUM, ABI::Windows::UI::Text::FontStretch_Normal);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_SEMI_EXPANDED, ABI::Windows::UI::Text::FontStretch_SemiExpanded);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_EXPANDED, ABI::Windows::UI::Text::FontStretch_Expanded);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_EXTRA_EXPANDED, ABI::Windows::UI::Text::FontStretch_ExtraExpanded);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STRETCH_ULTRA_EXPANDED, ABI::Windows::UI::Text::FontStretch_UltraExpanded);

        return static_cast<DWRITE_FONT_STRETCH>(value);
    }


    inline ABI::Windows::UI::Text::FontStretch ToWindowsFontStretch(DWRITE_FONT_STRETCH value)
    {
        // static_asserts in ToFontStretch validate that this cast is ok
        return static_cast<ABI::Windows::UI::Text::FontStretch>(value);
    }


    inline DWRITE_FONT_STYLE ToFontStyle(ABI::Windows::UI::Text::FontStyle value)
    {
        CHECK_ENUM_MEMBER(DWRITE_FONT_STYLE_NORMAL, ABI::Windows::UI::Text::FontStyle_Normal);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STYLE_OBLIQUE, ABI::Windows::UI::Text::FontStyle_Oblique);
        CHECK_ENUM_MEMBER(DWRITE_FONT_STYLE_ITALIC, ABI::Windows::UI::Text::FontStyle_Italic);

        return static_cast<DWRITE_FONT_STYLE>(value);
    }


    inline ABI::Windows::UI::Text::FontStyle ToWindowsFontStyle(DWRITE_FONT_STYLE value)
    {
        // static_asserts in ToFontStyle validate that this cast is ok
        return static_cast<ABI::Windows::UI::Text::FontStyle>(value);
    }


    inline DWRITE_FONT_WEIGHT ToFontWeight(ABI::Windows::UI::Text::FontWeight value)
    {
        return static_cast<DWRITE_FONT_WEIGHT>(value.Weight);
    }


    inline ABI::Windows::UI::Text::FontWeight ToWindowsFontWeight(DWRITE_FONT_WEIGHT value)
    {
        return ABI::Windows::UI::Text::FontWeight{ static_cast<uint16_t>(value) };
    }


    inline DWRITE_PARAGRAPH_ALIGNMENT ToParagraphAlignment(CanvasVerticalAlignment value)
    {
        CHECK_ENUM_MEMBER(DWRITE_PARAGRAPH_ALIGNMENT_NEAR, CanvasVerticalAlignment::Top);
        CHECK_ENUM_MEMBER(DWRITE_PARAGRAPH_ALIGNMENT_FAR, CanvasVerticalAlignment::Bottom);
        CHECK_ENUM_MEMBER(DWRITE_PARAGRAPH_ALIGNMENT_CENTER, CanvasVerticalAlignment::Center);

        return static_cast<DWRITE_PARAGRAPH_ALIGNMENT>(value);
    }


    inline CanvasVerticalAlignment ToCanvasVerticalAlignment(DWRITE_PARAGRAPH_ALIGNMENT value)
    {
        // static_asserts in ToParagraphAlignment validate that this cast is ok
        return static_cast<CanvasVerticalAlignment>(value);
    }


    inline DWRITE_TEXT_ALIGNMENT ToTextAlignment(CanvasHorizontalAlignment value)
    {
        CHECK_ENUM_MEMBER(DWRITE_TEXT_ALIGNMENT_LEADING, CanvasHorizontalAlignment::Left);
        CHECK_ENUM_MEMBER(DWRITE_TEXT_ALIGNMENT_TRAILING, CanvasHorizontalAlignment::Right);
        CHECK_ENUM_MEMBER(DWRITE_TEXT_ALIGNMENT_CENTER, CanvasHorizontalAlignment::Center);
        CHECK_ENUM_MEMBER(DWRITE_TEXT_ALIGNMENT_JUSTIFIED, CanvasHorizontalAlignment::Justified);

        return static_cast<DWRITE_TEXT_ALIGNMENT>(value);
    }


    inline CanvasHorizontalAlignment ToCanvasHorizontalAlignment(DWRITE_TEXT_ALIGNMENT value)
    {
        // static_asserts in ToTextAlignment validate that this case is ok
        return static_cast<CanvasHorizontalAlignment>(value);
    }


    inline DWRITE_TRIMMING_GRANULARITY ToTrimmingGranularity(CanvasTextTrimmingGranularity value)
    {
        CHECK_ENUM_MEMBER(DWRITE_TRIMMING_GRANULARITY_NONE, CanvasTextTrimmingGranularity::None);
        CHECK_ENUM_MEMBER(DWRITE_TRIMMING_GRANULARITY_CHARACTER, CanvasTextTrimmingGranularity::Character);
        CHECK_ENUM_MEMBER(DWRITE_TRIMMING_GRANULARITY_WORD, CanvasTextTrimmingGranularity::Word);

        return static_cast<DWRITE_TRIMMING_GRANULARITY>(value);
    }


    inline CanvasTextTrimmingGranularity ToCanvasTextTrimmingGranularity(DWRITE_TRIMMING_GRANULARITY value)
    {
        // static_asserts in ToTrimmingGranularity validate that this case is ok
        return static_cast<CanvasTextTrimmingGranularity>(value);
    }


    inline DWRITE_VERTICAL_GLYPH_ORIENTATION ToVerticalGlyphOrientation(CanvasVerticalGlyphOrientation value)
    {
        CHECK_ENUM_MEMBER(DWRITE_VERTICAL_GLYPH_ORIENTATION_DEFAULT, CanvasVerticalGlyphOrientation::Default);
        CHECK_ENUM_MEMBER(DWRITE_VERTICAL_GLYPH_ORIENTATION_STACKED, CanvasVerticalGlyphOrientation::Stacked);

        return static_cast<DWRITE_VERTICAL_GLYPH_ORIENTATION>(value);
    }


    inline CanvasVerticalGlyphOrientation ToCanvasVerticalGlyphOrientation(DWRITE_VERTICAL_GLYPH_ORIENTATION value)
    {
        // static_asserts in ToVerticalGlyphOrientation validate that this case is ok
        return static_cast<CanvasVerticalGlyphOrientation>(value);
    }


    inline DWRITE_OPTICAL_ALIGNMENT ToOpticalAlignment(CanvasOpticalAlignment value)
    {
        CHECK_ENUM_MEMBER(DWRITE_OPTICAL_ALIGNMENT_NONE, CanvasOpticalAlignment::Default);
        CHECK_ENUM_MEMBER(DWRITE_OPTICAL_ALIGNMENT_NO_SIDE_BEARINGS, CanvasOpticalAlignment::NoSideBearings);

        return static_cast<DWRITE_OPTICAL_ALIGNMENT>(value);
    }


    inline CanvasOpticalAlignment ToCanvasOpticalAlignment(DWRITE_OPTICAL_ALIGNMENT value)
    {
        // static_asserts in ToOpticalAlignment validate that this case is ok
        return static_cast<CanvasOpticalAlignment>(value);
    }


    inline WinString ToCanvasTrimmingDelimiter(uint32_t value)
    {
        // TODO #1658: Do the unicode conversion properly.
        // http://www.unicode.org/faq/utf_bom.html#utf16-3.  This code needs its
        // own set of tests.

        WinStringBuilder builder;

        if (value == 0)
        {
            return WinString();
        }
        else if (value <= 0x0000FFFF)
        {
            auto buffer = builder.Allocate(1);
            buffer[0] = static_cast<wchar_t>(value);
        }
        else
        {
            auto buffer = builder.Allocate(2);
            *(reinterpret_cast<uint32_t*>(buffer)) = value;
        }

        return builder.Get();
    }


    inline uint32_t ToTrimmingDelimiter(WinString const& value)
    {
        // TODO #1658: Do the unicode conversion properly.
        // http://www.unicode.org/faq/utf_bom.html#utf16-3.  This code needs its
        // own set of tests.

        uint32_t sourceStringLength = 0;
        auto sourceString = WindowsGetStringRawBuffer(value, &sourceStringLength);

        if (sourceStringLength == 0)
        {
            return 0;
        }
        else if (sourceStringLength == 1)
        {
            return sourceString[0];
        }
        else
        {
            return (sourceString[0] << 16) | sourceString[1];
        }
    }


    inline DWRITE_WORD_WRAPPING ToWordWrapping(CanvasWordWrapping value)
    {
        CHECK_ENUM_MEMBER(DWRITE_WORD_WRAPPING_WRAP, CanvasWordWrapping::Wrap);
        CHECK_ENUM_MEMBER(DWRITE_WORD_WRAPPING_NO_WRAP, CanvasWordWrapping::NoWrap);
        CHECK_ENUM_MEMBER(DWRITE_WORD_WRAPPING_EMERGENCY_BREAK, CanvasWordWrapping::EmergencyBreak);
        CHECK_ENUM_MEMBER(DWRITE_WORD_WRAPPING_WHOLE_WORD, CanvasWordWrapping::WholeWord);
        CHECK_ENUM_MEMBER(DWRITE_WORD_WRAPPING_CHARACTER, CanvasWordWrapping::Character);

        return static_cast<DWRITE_WORD_WRAPPING>(value);
    }


    inline CanvasWordWrapping ToCanvasWordWrapping(DWRITE_WORD_WRAPPING value)
    {
        // static_asserts in ToWordWrapping validate that this cast is ok
        return static_cast<CanvasWordWrapping>(value);
    }

    //
    // Parameter validation functions
    //

    template<typename T>
    inline void ThrowIfInvalid(T)
    {
        static_assert(false, "Specialization required");
    }

    template<>
    inline void ThrowIfInvalid(CanvasTextDirection value)
    {
        switch (value)
        {
        case CanvasTextDirection::LeftToRightThenTopToBottom:
        case CanvasTextDirection::RightToLeftThenTopToBottom:
        case CanvasTextDirection::LeftToRightThenBottomToTop:
        case CanvasTextDirection::RightToLeftThenBottomToTop:
        case CanvasTextDirection::TopToBottomThenLeftToRight:
        case CanvasTextDirection::BottomToTopThenLeftToRight:
        case CanvasTextDirection::TopToBottomThenRightToLeft:
        case CanvasTextDirection::BottomToTopThenRightToLeft:
            return;
        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(CanvasVerticalAlignment value)
    {
        switch (value)
        {
        case CanvasVerticalAlignment::Top:
        case CanvasVerticalAlignment::Bottom:
        case CanvasVerticalAlignment::Center:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(CanvasHorizontalAlignment value)
    {
        switch (value)
        {
        case CanvasHorizontalAlignment::Left:
        case CanvasHorizontalAlignment::Right:
        case CanvasHorizontalAlignment::Center:
        case CanvasHorizontalAlignment::Justified:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(ABI::Windows::UI::Text::ParagraphAlignment value)
    {
        switch (value)
        {
        case ABI::Windows::UI::Text::ParagraphAlignment_Undefined:
        case ABI::Windows::UI::Text::ParagraphAlignment_Left:
        case ABI::Windows::UI::Text::ParagraphAlignment_Center:
        case ABI::Windows::UI::Text::ParagraphAlignment_Right:
        case ABI::Windows::UI::Text::ParagraphAlignment_Justify:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(CanvasWordWrapping value)
    {
        switch (value)
        {
        case CanvasWordWrapping::Wrap:
        case CanvasWordWrapping::NoWrap:
        case CanvasWordWrapping::EmergencyBreak:
        case CanvasWordWrapping::WholeWord:
        case CanvasWordWrapping::Character:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(CanvasTextTrimmingGranularity value)
    {
        switch (value)
        {
        case CanvasTextTrimmingGranularity::None:
        case CanvasTextTrimmingGranularity::Character:
        case CanvasTextTrimmingGranularity::Word:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

#if WINVER > _WIN32_WINNT_WINBLUE
    template<>
    inline void ThrowIfInvalid(CanvasLineSpacingMode value)
    {
        switch (value)
        {
        case CanvasLineSpacingMode::Default:
        case CanvasLineSpacingMode::Uniform:
        case CanvasLineSpacingMode::Proportional:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    inline CanvasLineSpacingMode ToCanvasLineSpacingMode(DWRITE_LINE_SPACING_METHOD value)
    {
        // static_asserts in ToLineSpacingMethod validate that this cast is ok
        return static_cast<CanvasLineSpacingMode>(value);
    }

    inline DWRITE_LINE_SPACING_METHOD ToLineSpacingMethod(CanvasLineSpacingMode value)
    {
        CHECK_ENUM_MEMBER(DWRITE_LINE_SPACING_METHOD_DEFAULT, CanvasLineSpacingMode::Default);
        CHECK_ENUM_MEMBER(DWRITE_LINE_SPACING_METHOD_UNIFORM, CanvasLineSpacingMode::Uniform);
        CHECK_ENUM_MEMBER(DWRITE_LINE_SPACING_METHOD_PROPORTIONAL, CanvasLineSpacingMode::Proportional);

        return static_cast<DWRITE_LINE_SPACING_METHOD>(value);
    }
#endif    

    template<>
    inline void ThrowIfInvalid(CanvasVerticalGlyphOrientation value)
    {
        switch (value)
        {
        case CanvasVerticalGlyphOrientation::Default:
        case CanvasVerticalGlyphOrientation::Stacked:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(CanvasOpticalAlignment value)
    {
        switch (value)
        {
        case CanvasOpticalAlignment::Default:
        case CanvasOpticalAlignment::NoSideBearings:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    template<>
    inline void ThrowIfInvalid(CanvasTrimmingSign value)
    {
        switch (value)
        {
        case CanvasTrimmingSign::None:
        case CanvasTrimmingSign::Ellipsis:
            return;

        default:
            ThrowHR(E_INVALIDARG);
        }
    }

    inline void ThrowIfInvalidTrimmingDelimiter(HSTRING value)
    {
        // The delimiter must be a single code point and so cannot be more than
        // 2 UTF-16 code units long
        if (WindowsGetStringLen(value) > 2)
            ThrowHR(E_INVALIDARG);
    }
    
    template <class GET_ATTRIBUTE_LENGTH_METHOD, class GET_ATTRIBUTE_METHOD>
    inline WinString GetStringAttributeFromTextFormat(
        GET_ATTRIBUTE_LENGTH_METHOD fnGetLength,
        GET_ATTRIBUTE_METHOD fnGet,
        IDWriteTextFormat* format)
    {
        WinStringBuilder stringBuilder;
        uint32_t attributeLength = (format->*fnGetLength)();
        attributeLength++; // Account for null terminator

        auto buffer = stringBuilder.Allocate(attributeLength);
        ThrowIfFailed((format->*fnGet)(buffer, attributeLength));
        return stringBuilder.Get();
    }

    inline WinString GetFontFamilyName(IDWriteTextFormat* format)
    {
        return GetStringAttributeFromTextFormat(
            &IDWriteTextFormat::GetFontFamilyNameLength,
            &IDWriteTextFormat::GetFontFamilyName,
            format);
    }

    inline WinString GetLocaleName(IDWriteTextFormat* format)
    {
        return GetStringAttributeFromTextFormat(
            &IDWriteTextFormat::GetLocaleNameLength,
            &IDWriteTextFormat::GetLocaleName,
            format);
    }

    template<typename DWriteContainerType>
    inline WinString GetLocaleFromLocalizedStrings(
        int32_t stringIndex,
        ComPtr<DWriteContainerType> const& localizedStrings)
    {
        WinStringBuilder stringBuilder;
        uint32_t attributeLength;
        ThrowIfFailed(localizedStrings->GetLocaleNameLength(stringIndex, &attributeLength));
        attributeLength++; // Account for null terminator

        auto buffer = stringBuilder.Allocate(attributeLength);
        ThrowIfFailed(localizedStrings->GetLocaleName(stringIndex, buffer, attributeLength));
        return stringBuilder.Get();
    }

    template<typename DWriteContainerType>
    inline WinString GetTextFromLocalizedStrings(
        int32_t stringIndex,
        ComPtr<DWriteContainerType> const& localizedStrings)
    {
        WinStringBuilder stringBuilder;
        uint32_t attributeLength;
        ThrowIfFailed(localizedStrings->GetStringLength(stringIndex, &attributeLength));
        attributeLength++; // Account for null terminator

        auto buffer = stringBuilder.Allocate(attributeLength);
        ThrowIfFailed(localizedStrings->GetString(stringIndex, buffer, attributeLength));
        return stringBuilder.Get();
    }

    inline std::pair<WinString, WinString> GetUriAndFontFamily(WinString const& fontFamilyName)
    {
        auto beginIt = begin(fontFamilyName);
        auto endIt = end(fontFamilyName);

        auto hashPos = std::find(beginIt, endIt, L'#');

        if (hashPos == endIt)
            return std::make_pair(WinString(), fontFamilyName);
        else
            return std::make_pair(WinString(beginIt, hashPos), WinString(hashPos + 1, endIt));
    }

#if WINVER > _WIN32_WINNT_WINBLUE

    inline CanvasTextRenderingMode ToCanvasTextRenderingMode(DWRITE_RENDERING_MODE1 const& value)
    {
        // static_asserts in ToDWriteRenderingMode validate that this cast is ok
        return static_cast<CanvasTextRenderingMode>(value);
    }

    inline DWRITE_RENDERING_MODE1 ToDWriteRenderingMode(CanvasTextRenderingMode value)
    {
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_DEFAULT, CanvasTextRenderingMode::Default);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_ALIASED, CanvasTextRenderingMode::Aliased);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_GDI_CLASSIC, CanvasTextRenderingMode::GdiClassic);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_GDI_NATURAL, CanvasTextRenderingMode::GdiNatural);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_NATURAL, CanvasTextRenderingMode::Natural);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_NATURAL_SYMMETRIC, CanvasTextRenderingMode::NaturalSymmetric);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_OUTLINE, CanvasTextRenderingMode::Outline);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE1_NATURAL_SYMMETRIC_DOWNSAMPLED, CanvasTextRenderingMode::NaturalSymmetricDownsampled);
        return static_cast<DWRITE_RENDERING_MODE1>(value);
    }
#else

    inline CanvasTextRenderingMode ToCanvasTextRenderingMode(DWRITE_RENDERING_MODE const& value)
    {
        // static_asserts in ToDWriteRenderingMode validate that this cast is ok
        return static_cast<CanvasTextRenderingMode>(value);
    }

    inline DWRITE_RENDERING_MODE ToDWriteRenderingMode(CanvasTextRenderingMode value)
    {
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_DEFAULT, CanvasTextRenderingMode::Default);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_ALIASED, CanvasTextRenderingMode::Aliased);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_GDI_CLASSIC, CanvasTextRenderingMode::GdiClassic);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_GDI_NATURAL, CanvasTextRenderingMode::GdiNatural);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_NATURAL, CanvasTextRenderingMode::Natural);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_NATURAL_SYMMETRIC, CanvasTextRenderingMode::NaturalSymmetric);
        CHECK_ENUM_MEMBER(DWRITE_RENDERING_MODE_OUTLINE, CanvasTextRenderingMode::Outline);
        return static_cast<DWRITE_RENDERING_MODE>(value);
    }
#endif

    inline CanvasTextGridFit ToCanvasTextGridFit(DWRITE_GRID_FIT_MODE value)
    {
        // static_asserts in ToDWriteGridFitMode validate that this cast is ok
        return static_cast<CanvasTextGridFit>(value);
    }

    inline DWRITE_GRID_FIT_MODE ToDWriteGridFitMode(CanvasTextGridFit value)
    {
        CHECK_ENUM_MEMBER(DWRITE_GRID_FIT_MODE_DEFAULT, CanvasTextGridFit::Default);
        CHECK_ENUM_MEMBER(DWRITE_GRID_FIT_MODE_DISABLED, CanvasTextGridFit::Disable);
        CHECK_ENUM_MEMBER(DWRITE_GRID_FIT_MODE_ENABLED, CanvasTextGridFit::Enable);
        return static_cast<DWRITE_GRID_FIT_MODE>(value);
    }

    inline DWRITE_MEASURING_MODE ToDWriteMeasuringMode(CanvasTextMeasuringMode value)
    {
        CHECK_ENUM_MEMBER(DWRITE_MEASURING_MODE_NATURAL, CanvasTextMeasuringMode::Natural);
        CHECK_ENUM_MEMBER(DWRITE_MEASURING_MODE_GDI_CLASSIC, CanvasTextMeasuringMode::GdiClassic);
        CHECK_ENUM_MEMBER(DWRITE_MEASURING_MODE_GDI_NATURAL, CanvasTextMeasuringMode::GdiNatural);
        return static_cast<DWRITE_MEASURING_MODE>(value);
    }

    inline DWRITE_OUTLINE_THRESHOLD ToDWriteOutlineThreshold(CanvasAntialiasing value)
    {
        CHECK_ENUM_MEMBER(DWRITE_OUTLINE_THRESHOLD_ANTIALIASED, CanvasAntialiasing::Antialiased);
        CHECK_ENUM_MEMBER(DWRITE_OUTLINE_THRESHOLD_ALIASED, CanvasAntialiasing::Aliased);
        return static_cast<DWRITE_OUTLINE_THRESHOLD>(value);
    }
    
    inline DWRITE_FONT_FACE_TYPE ToDWriteFontFaceType(CanvasFontFileFormatType value)
    {
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_CFF, CanvasFontFileFormatType::Cff);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_TRUETYPE, CanvasFontFileFormatType::TrueType);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_TRUETYPE_COLLECTION, CanvasFontFileFormatType::TrueTypeCollection);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_TYPE1, CanvasFontFileFormatType::Type1);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_VECTOR, CanvasFontFileFormatType::Vector);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_BITMAP, CanvasFontFileFormatType::Bitmap);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_UNKNOWN, CanvasFontFileFormatType::Unknown);
        CHECK_ENUM_MEMBER(DWRITE_FONT_FACE_TYPE_RAW_CFF, CanvasFontFileFormatType::RawCff);
        return static_cast<DWRITE_FONT_FACE_TYPE>(value);
    }

    inline CanvasFontFileFormatType ToCanvasFontFileFormatType(DWRITE_FONT_FACE_TYPE value)
    {
        // static_asserts in ToDWriteFontFaceType validate that this cast is ok
        return static_cast<CanvasFontFileFormatType>(value);
    }

    inline DWRITE_FONT_SIMULATIONS ToDWriteFontSimulations(CanvasFontSimulations value)
    {
        CHECK_ENUM_MEMBER(DWRITE_FONT_SIMULATIONS_NONE, CanvasFontSimulations::None);
        CHECK_ENUM_MEMBER(DWRITE_FONT_SIMULATIONS_BOLD, CanvasFontSimulations::Bold);
        CHECK_ENUM_MEMBER(DWRITE_FONT_SIMULATIONS_OBLIQUE, CanvasFontSimulations::Oblique);
        return static_cast<DWRITE_FONT_SIMULATIONS>(value);
    }

    inline CanvasFontSimulations ToCanvasFontSimulations(DWRITE_FONT_SIMULATIONS value)
    {
        // static_asserts in ToDWriteFontSimulations validate that this cast is ok
        return static_cast<CanvasFontSimulations>(value);
    }

    inline DWRITE_INFORMATIONAL_STRING_ID ToDWriteInformationalStringId(CanvasFontInformation value)
    {
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_NONE, CanvasFontInformation::None);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_COPYRIGHT_NOTICE, CanvasFontInformation::CopyrightNotice);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_VERSION_STRINGS, CanvasFontInformation::VersionStrings);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_TRADEMARK, CanvasFontInformation::Trademark);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_MANUFACTURER, CanvasFontInformation::Manufacturer);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_DESIGNER, CanvasFontInformation::Designer);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_DESIGNER_URL, CanvasFontInformation::DesignerUrl);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_DESCRIPTION, CanvasFontInformation::Description);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_FONT_VENDOR_URL, CanvasFontInformation::FontVendorUrl);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_LICENSE_DESCRIPTION, CanvasFontInformation::LicenseDescription);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_LICENSE_INFO_URL, CanvasFontInformation::LicenseInfoUrl);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_WIN32_FAMILY_NAMES, CanvasFontInformation::Win32FamilyNames);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_WIN32_SUBFAMILY_NAMES, CanvasFontInformation::Win32SubfamilyNames);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_PREFERRED_FAMILY_NAMES, CanvasFontInformation::PreferredFamilyNames);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_PREFERRED_SUBFAMILY_NAMES, CanvasFontInformation::PreferredSubfamilyNames);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_SAMPLE_TEXT, CanvasFontInformation::SampleText);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_FULL_NAME, CanvasFontInformation::FullName);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_NAME, CanvasFontInformation::PostscriptName);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_POSTSCRIPT_CID_NAME, CanvasFontInformation::PostscriptCidName);
#if WINVER > _WIN32_WINNT_WINBLUE
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_WWS_FAMILY_NAME, CanvasFontInformation::WwsFamilyName);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_DESIGN_SCRIPT_LANGUAGE_TAG, CanvasFontInformation::DesignScriptLanguageTag);
        CHECK_ENUM_MEMBER(DWRITE_INFORMATIONAL_STRING_SUPPORTED_SCRIPT_LANGUAGE_TAG, CanvasFontInformation::SupportedScriptLanguageTag);
#endif
        return static_cast<DWRITE_INFORMATIONAL_STRING_ID>(value);
    }

#if WINVER > _WIN32_WINNT_WINBLUE
    inline DWRITE_FONT_PROPERTY_ID ToDWriteFontPropertyId(CanvasFontPropertyIdentifier value)
    {
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_NONE, CanvasFontPropertyIdentifier::None);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_FAMILY_NAME, CanvasFontPropertyIdentifier::FamilyName);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_PREFERRED_FAMILY_NAME, CanvasFontPropertyIdentifier::PreferredFamilyName);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_FACE_NAME, CanvasFontPropertyIdentifier::FaceName);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_FULL_NAME, CanvasFontPropertyIdentifier::FullName);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_WIN32_FAMILY_NAME, CanvasFontPropertyIdentifier::Win32FamilyName);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_POSTSCRIPT_NAME, CanvasFontPropertyIdentifier::PostscriptName);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_DESIGN_SCRIPT_LANGUAGE_TAG, CanvasFontPropertyIdentifier::DesignScriptLanguageTag);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_SUPPORTED_SCRIPT_LANGUAGE_TAG, CanvasFontPropertyIdentifier::SupportedScriptLanguageTag);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_SEMANTIC_TAG, CanvasFontPropertyIdentifier::SemanticTag);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_WEIGHT, CanvasFontPropertyIdentifier::Weight);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_STRETCH, CanvasFontPropertyIdentifier::Stretch);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_STYLE, CanvasFontPropertyIdentifier::Style);
        CHECK_ENUM_MEMBER(DWRITE_FONT_PROPERTY_ID_TOTAL, CanvasFontPropertyIdentifier::Total);

        return static_cast<DWRITE_FONT_PROPERTY_ID>(value);
    }
#endif

    //
    // The localizedStrings are allowed to be null. If they are, this returns an empty map view.
    //
    inline void CopyLocalizedStringsToMapView(ComPtr<IDWriteLocalizedStrings> const& localizedStrings, IMapView<HSTRING, HSTRING>** values)
    {
        auto map = Make<Map<HSTRING, HSTRING>>();
        CheckMakeResult(map);

        if (localizedStrings)
        {
            const uint32_t stringCount = localizedStrings->GetCount();

            for (uint32_t i = 0; i < stringCount; ++i)
            {
                auto name = GetTextFromLocalizedStrings(i, localizedStrings);

                auto locale = GetLocaleFromLocalizedStrings(i, localizedStrings);

                boolean unused;
                ThrowIfFailed(map->Insert(locale, name, &unused));
            }
        }

        ThrowIfFailed(map->GetView(values));
    }
   
}}}}}
