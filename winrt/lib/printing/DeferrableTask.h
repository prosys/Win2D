// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Licensed under the MIT License. See LICENSE.txt in the project root for license information.

#pragma once

#include "CanvasPrintDeferral.h"

using namespace ABI::Microsoft::Graphics::Canvas::Printing;

class DeferrableTaskScheduler;

class DeferrableTask;
typedef std::shared_ptr<DeferrableTask> DeferrableTaskPtr;
typedef std::function<void(DeferrableTaskPtr)> DeferrableFn;

class DeferrableTask : public std::enable_shared_from_this<DeferrableTask>
{
    DeferrableTaskScheduler* m_owner;
    bool m_deferred;

    DeferrableFn m_code;
    std::function<void()> m_completionFn;
    
public:
    DeferrableTask(DeferrableTaskScheduler* owner, DeferrableFn fn);

    void Invoke();

    void SetCompletionFn(std::function<void()>&& fn);
    
    void NonDeferredComplete();
    void DeferredComplete();
    void Completed();
    
    ComPtr<CanvasPrintDeferral> GetDeferral();

    DeferrableTask(DeferrableTask const&) = delete;
    DeferrableTask(DeferrableTask&&) = delete;
};

