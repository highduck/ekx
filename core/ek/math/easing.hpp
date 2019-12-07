#pragma once

namespace ek {

class basic_ease_t {
protected:
    basic_ease_t() = default;

    virtual ~basic_ease_t();

public:
    virtual float calculate(float t) const = 0;
};

class Back : public basic_ease_t {
public:
    static constexpr float DEFAULT_OVERSHOOT = 1.70158f;

    float overshoot;

    explicit Back(float overshoot = DEFAULT_OVERSHOOT) noexcept
            : overshoot{overshoot} {

    }

    ~Back() override = default;

    float calculate(float t) const override;
};

class Bounce : public basic_ease_t {
public:
    static constexpr float B1 = 1.0f / 2.75f;
    static constexpr float B2 = 2.0f / 2.75f;
    static constexpr float B3 = 1.5f / 2.75f;
    static constexpr float B4 = 2.5f / 2.75f;
    static constexpr float B5 = 2.25f / 2.75f;
    static constexpr float B6 = 2.625f / 2.75f;

    Bounce() noexcept {}

    ~Bounce() override = default;

    float calculate(float t) const override;
};

class Circ : public basic_ease_t {
public:

    Circ() noexcept = default;

    ~Circ() override = default;

    float calculate(float t) const override;
};

class Elastic : public basic_ease_t {
public:
    static constexpr float DEFAULT_AMPLITUDE = 0.1f;
    static constexpr float DEFAULT_PERIOD = 0.4f;

    float amplitude;
    float period;

    explicit Elastic(float amplitude = DEFAULT_AMPLITUDE, float period = DEFAULT_PERIOD) noexcept : amplitude{
            amplitude},
                                                                                                    period{period} {}

    ~Elastic() override = default;

    float calculate(float t) const override;
};

class Expo : public basic_ease_t {
public:

    Expo() noexcept = default;

    ~Expo() override = default;

    float calculate(float t) const override;
};

class Linear : public basic_ease_t {
public:

    Linear() noexcept = default;

    ~Linear() override = default;

    float calculate(float t) const override;
};

class Polynomial : public basic_ease_t {
public:
    float degree;

    explicit Polynomial(float degree = 3.0f) noexcept : degree{degree} {}

    ~Polynomial() override = default;

    float calculate(float t) const override;
};

class Sine : public basic_ease_t {
public:
    Sine() noexcept = default;

    ~Sine() override = default;

    float calculate(float t) const override;
};

class Step : public basic_ease_t {
public:
    Step() noexcept = default;

    ~Step() override = default;

    float calculate(float t) const override;
};

template<class T>
class EaseInOut : public basic_ease_t {
public:
    T eq;

    explicit EaseInOut(T eq) noexcept : eq{eq} {}

    ~EaseInOut() override = default;

    float calculate(float t) const override {
        return t < 0.5f ? 0.5f * eq.calculate(t * 2.0f) : 1.0f - 0.5f * eq.calculate(2.0f - t * 2.0f);
    }
};

template<class T>
class EaseOut : public basic_ease_t {
public:
    T eq;

    explicit EaseOut(T eq) noexcept : eq{eq} {}

    ~EaseOut() override = default;

    float calculate(float t) const override {
        return 1.0f - eq.calculate(1.0f - t);
    }
};

template<class T>
class EaseOutIn : public basic_ease_t {
public:
    T eq;

    explicit EaseOutIn(T eq) noexcept : eq{eq} {}

    ~EaseOutIn() override = default;

    float calculate(float t) const override {
        return t < 0.5f ? 0.5f * (1.0f - eq.calculate(1.0f - t * 2.0f)) : 0.5f + 0.5f * eq.calculate(t * 2.0f - 1.0f);
    }
};

namespace easing {

static const Polynomial P2_IN(2);
static const EaseOut P2_OUT(P2_IN);

static const Polynomial P3_IN(3);
static const EaseOut P3_OUT(P3_IN);
static const EaseInOut P3_IN_OUT(P3_IN);

static const Polynomial P4_IN(4);

static const Elastic ELASTIC_IN;
static const EaseOut ELASTIC_OUT(ELASTIC_IN);

static const Circ CIRC_IN;
static const EaseOut CIRC_OUT(CIRC_IN);
static const EaseInOut CIRC_IN_OUT(CIRC_IN);

static const Bounce BOUNCE_IN;
static const EaseOut BOUNCE_OUT(BOUNCE_IN);

static const Back BACK_IN;
static const EaseOut BACK_OUT(BACK_IN);

}
}
