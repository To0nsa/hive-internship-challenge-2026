#pragma once

#include "states/IState.h"

#include <cassert>
#include <memory>
#include <typeinfo>
#include <vector>

class StateStack {
  public:
    IState* getCurrentState() { return m_states.empty() ? nullptr : m_states.back().get(); }

    template <typename T, typename... Args> void push(Args&&... args) {
        std::unique_ptr<IState> state = std::make_unique<T>(*this, std::forward<Args>(args)...);
        if (!state->init())
            throw std::runtime_error(std::string("Failed to initialize state: ") +
                                     typeid(T).name());

        m_states.push_back(std::move(state));
    }

    std::unique_ptr<IState> pop() {
        std::unique_ptr<IState> pState = std::move(m_states.back());
        m_states.pop_back();
        return pState;
    }

    void requestPop() { m_requestPopCount++; }

    void performPendingPops() {
        while (m_requestPopCount > 0) {
            --m_requestPopCount;
            assert(!m_states.empty());
            if (m_states.empty())
                break;
            m_states.pop_back();
        }
    }

  private:
    std::vector<std::unique_ptr<IState>> m_states;
    size_t                               m_requestPopCount = 0;
};
