#pragma once

#include "Stopwatch.h"

class GameSession {
  public:
    void start() {
        m_clock.reset();
        m_score      = 0;
        m_scoreAccum = 0.f;
    }

    void update(float dt) { m_clock.tick(dt); }

    float elapsedSeconds() const { return m_clock.getElapsed(); }

    void addScore(int pts) {
        if (pts <= 0)
            return;
        m_score += pts;
    }

    void addScorePerSecond(float dt, int ratePerSecond) {
        m_scoreAccum += dt;
        while (m_scoreAccum >= 1.f) {
            m_score += ratePerSecond;
            m_scoreAccum -= 1.f;
        }
    }

    int score() const { return m_score; }

  private:
    Stopwatch m_clock;
    int       m_score      = 0;
    float     m_scoreAccum = 0.f;
    // Level data can be added later.
};
