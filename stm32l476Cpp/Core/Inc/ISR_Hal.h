/*
 * ISR_Hal.h (heap-free, simplified)
 *
 *  Created on: Oct 29, 2025
 *      Author: FA
 *
 *  Description:
 *    - Static-capacity ISR registry (no heap, no STL)
 *    - Fixed-size array of pointers
 *    - O(1) remove using swap-with-last
 *    - Safe for bare-metal embedded systems
 */

#ifndef INC_ISR_HAL_H_
#define INC_ISR_HAL_H_

#include "main.h"
#include <stddef.h>
#include <stdint.h>

/* ------------------------------------------------------------
 * ISR<T, MAX>
 * Heap-free registry for ISR-capable objects.
 *
 * Template parameters:
 *   T   : object type (e.g., ButtonISR)
 *   MAX : maximum number of registered objects
 *
 * Behavior:
 *   - No dynamic allocation
 *   - add(): adds object if not already present and space available
 *   - remove(): deletes by swapping with last element
 *   - get(): returns object pointer by index
 * ---------------------------------------------------------- */
template <class T, size_t MAX>
class ISR {
public:
    ISR() : count_(0) {
        // Initialize list with nullptrs (optional)
        for (size_t i = 0; i < MAX; ++i) {
            list_[i] = nullptr;
        }
    }

    ~ISR() = default;

    // ------------------------------------------------------
    // Add an object pointer to the registry
    // ------------------------------------------------------
    bool add(T* obj) {
        if (obj == nullptr) return false;

        // Check for duplicates
        for (size_t i = 0; i < count_; ++i) {
            if (list_[i] == obj) {
                return true;  // already in list
            }
        }

        // Check for available space
        if (count_ >= MAX) {
            return false; // list full
        }

        list_[count_++] = obj;
        return true;
    }

    // ------------------------------------------------------
    // Remove an object pointer (O(1))
    // ------------------------------------------------------
    bool remove(T* obj) {
        if (obj == nullptr) return false;

        for (size_t i = 0; i < count_; ++i) {
            if (list_[i] == obj) {
                // Replace with last element
                list_[i] = list_[count_ - 1];
                list_[count_ - 1] = nullptr;
                --count_;
                return true;
            }
        }
        return false; // not found
    }

    // ------------------------------------------------------
    // Get object by index (nullptr if invalid)
    // ------------------------------------------------------
    T* get(size_t index) const {
        if (index >= count_) return nullptr;
        return list_[index];
    }

    // ------------------------------------------------------
    // Get current number of elements
    // ------------------------------------------------------
    size_t size() const { return count_; }

    // ------------------------------------------------------
    // Get compile-time capacity
    // ------------------------------------------------------
    static constexpr size_t capacity() { return MAX; }

private:
    T*     list_[MAX];  // fixed-size array (no heap)
    size_t count_;      // number of active elements
};

#endif /* INC_ISR_HAL_H_ */

