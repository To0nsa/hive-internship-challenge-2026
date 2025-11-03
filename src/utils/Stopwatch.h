#pragma once

#include <algorithm>

class Stopwatch {
  public:
    void reset() { m_elapsed = 0.f; }
    void tick(float dt) {
        if (!m_paused)
            m_elapsed += dt * m_scale;
    }
    void  setPaused(bool paused) { m_paused = paused; }
    bool  isPaused() const { return m_paused; }
    void  setScale(float scale) { m_scale = scale; }
    float getElapsed() const { return m_elapsed; }

  private:
    float m_elapsed = 0.f;
    float m_scale   = 1.f;
    bool  m_paused  = false;
};
