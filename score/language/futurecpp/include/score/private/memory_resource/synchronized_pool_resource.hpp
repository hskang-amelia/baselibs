/********************************************************************************
 * Copyright (c) 2026 Contributors to the Eclipse Foundation
 *
 * See the NOTICE file(s) distributed with this work for additional
 * information regarding copyright ownership.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Apache License Version 2.0 which is available at
 * https://www.apache.org/licenses/LICENSE-2.0
 *
 * SPDX-License-Identifier: Apache-2.0
 ********************************************************************************/

///
/// \file
/// \copyright Copyright (c) 2026 Contributors to the Eclipse Foundation
///
/// \brief Score.Futurecpp.Pmr.MemoryResource component
///

// IWYU pragma: private

#ifndef SCORE_LANGUAGE_FUTURECPP_PRIVATE_MEMORY_RESOURCE_SYNCHRONIZED_POOL_RESOURCE_HPP
#define SCORE_LANGUAGE_FUTURECPP_PRIVATE_MEMORY_RESOURCE_SYNCHRONIZED_POOL_RESOURCE_HPP

#include <score/private/memory_resource/memory_resource.hpp>
#include <score/private/memory_resource/pool_options.hpp>
#include <score/private/memory_resource/unsynchronized_pool_resource.hpp>

#include <cstddef>
#include <mutex>

namespace score::cpp
{
namespace pmr
{

/// \brief A thread-safe memory resource that manages allocations in pools of different block sizes.
///
/// Behaves like score::cpp::pmr::unsynchronized_pool_resource, but may be accessed from multiple threads at the same
/// time: every allocation, deallocation and release() is serialized by an internal mutex.
///
/// \see https://en.cppreference.com/w/cpp/memory/synchronized_pool_resource
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions) Follows literaly the C++ standard
class synchronized_pool_resource : public memory_resource
{
public:
    /// \brief Constructs a synchronized_pool_resource using the specified upstream memory resource and tuned according
    /// to the specified options.
    ///
    /// \pre upstream != nullptr
    /// \see https://en.cppreference.com/w/cpp/memory/synchronized_pool_resource/synchronized_pool_resource
    synchronized_pool_resource(const pool_options& opts, memory_resource* const upstream) : pool_{opts, upstream} {}

    /// \brief Constructs a synchronized_pool_resource using default pool_options and
    /// score::cpp::pmr::get_default_resource() as the upstream memory resource.
    synchronized_pool_resource() : synchronized_pool_resource(pool_options{}, get_default_resource()) {}

    /// \brief Constructs a synchronized_pool_resource using the specified upstream memory resource and default
    /// pool_options.
    ///
    /// \pre upstream != nullptr
    explicit synchronized_pool_resource(memory_resource* const upstream)
        : synchronized_pool_resource{pool_options{}, upstream}
    {
    }

    /// \brief Constructs a synchronized_pool_resource tuned according to the specified options and using
    /// score::cpp::pmr::get_default_resource() as the upstream memory resource.
    explicit synchronized_pool_resource(const pool_options& opts)
        : synchronized_pool_resource{opts, get_default_resource()}
    {
    }

    synchronized_pool_resource(const synchronized_pool_resource&) = delete;
    synchronized_pool_resource& operator=(const synchronized_pool_resource&) = delete;

    /// \brief Destroys a synchronized_pool_resource, releasing all allocated memory.
    ~synchronized_pool_resource() override = default;

    /// \brief Releases all memory owned by this resource back to the upstream memory resource.
    ///
    /// \see https://en.cppreference.com/w/cpp/memory/synchronized_pool_resource/release
    void release()
    {
        const std::lock_guard<std::mutex> lock{mutex_};
        pool_.release();
    }

    /// \brief Returns a pointer to the upstream memory resource.
    memory_resource* upstream_resource() const { return pool_.upstream_resource(); }

    /// \brief Returns the options that control the pooling behavior of this resource.
    pool_options options() const { return pool_.options(); }

protected:
    /// \brief Allocates storage from the pools, serialized by the internal mutex.
    ///
    /// \pre alignment is a power of 2.
    void* do_allocate(const std::size_t bytes, const std::size_t alignment) override
    {
        const std::lock_guard<std::mutex> lock{mutex_};
        return pool_.allocate(bytes, alignment);
    }

    /// \brief Returns the memory at p to the pools, serialized by the internal mutex.
    ///
    /// \pre p has been allocated from this object with the specified bytes and alignment.
    void do_deallocate(void* const p, const std::size_t bytes, const std::size_t alignment) override
    {
        const std::lock_guard<std::mutex> lock{mutex_};
        pool_.deallocate(p, bytes, alignment);
    }

    /// \brief Compare for equality with another score::cpp::pmr::memory_resource.
    ///
    /// \return this == &other
    bool do_is_equal(const memory_resource& other) const noexcept override { return this == &other; }

private:
    unsynchronized_pool_resource pool_;
    std::mutex mutex_;
};

} // namespace pmr
} // namespace score::cpp

#endif // SCORE_LANGUAGE_FUTURECPP_PRIVATE_MEMORY_RESOURCE_SYNCHRONIZED_POOL_RESOURCE_HPP
