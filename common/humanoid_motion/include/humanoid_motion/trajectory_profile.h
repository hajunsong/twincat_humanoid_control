// humanoid_motion/trajectory_profile.h
//
// Pure C++ trajectory primitives extracted from the historical
// 31-motor reference module. Formulas, thresholds, and rounding are
// kept byte-identical to that reference so behaviour is preserved
// when call sites adopt these helpers.
//
// Real-time / cyclic safety:
//   - No dynamic allocation, no exceptions.
//   - No malloc/free/new/delete.
//   - No standard-library facilities that may allocate.
//   - No standard headers required.
//
// No TwinCAT / TcCOM / TMC / ADS / EtherCAT dependency.
#pragma once

namespace humanoid {
namespace motion {

// Hard cap on step counts produced by ComputeStepCount(). Identical to
// the reference module's kTrajMaxSteps.
constexpr int kTrajectoryMaxSteps = 10000;

// Horner evaluation of a fifth-order polynomial:
//   a0 + a1*t + a2*t^2 + a3*t^3 + a4*t^4 + a5*t^5
inline double Eval5th(double a0,
                      double a1,
                      double a2,
                      double a3,
                      double a4,
                      double a5,
                      double t)
{
    return a0 + t * (a1 + t * (a2 + t * (a3 + t * (a4 + t * a5))));
}

// Trapezoidal velocity profile with fifth-order polynomial blends in
// the acceleration and deceleration phases. The reference function
// returned the position via an output pointer; here it is returned by
// value because every code path produces a single scalar. Edge cases
// (the three "ts < 1e-9" early returns and the td floor at 1e-9) are
// preserved exactly. ta and t clamping policy stays in the caller.
inline double Trapezoidal5thAtTime(double x0,
                                   double xf,
                                   double tf,
                                   double ta,
                                   double t)
{
    double td = tf - ta;
    if (td < 1e-9)
        td = 1e-9;
    double vd = (xf - x0) / td;
    double xa = x0 + 0.5 * ta * vd;
    double xd = xf - 0.5 * ta * vd;

    double pos0, posf, vel0, velf, acc0, accf, ts;
    double a0, a1, a2, a3, a4, a5;

    if (t <= ta)
    {
        pos0 = x0;
        posf = xa;
        vel0 = 0;
        velf = vd;
        acc0 = 0;
        accf = 0;
        ts = ta;
        if (ts < 1e-9)
            return x0;
        a0 = pos0;
        a1 = vel0;
        a2 = acc0 / 2.0;
        a3 = (20.0 * (posf - pos0) - (8.0 * velf + 12.0 * vel0) * ts - (3.0 * acc0 - accf) * ts * ts) / (2.0 * ts * ts * ts);
        a4 = (30.0 * (pos0 - posf) + (14.0 * velf + 16.0 * vel0) * ts + (3.0 * acc0 - 2.0 * accf) * ts * ts) / (2.0 * ts * ts * ts * ts);
        a5 = (12.0 * (posf - pos0) - (6.0 * velf + 6.0 * vel0) * ts - (acc0 - accf) * ts * ts) / (2.0 * ts * ts * ts * ts * ts);
        return Eval5th(a0, a1, a2, a3, a4, a5, t);
    }
    if (t <= td)
    {
        pos0 = xa;
        posf = xd;
        vel0 = vd;
        velf = vd;
        acc0 = 0;
        accf = 0;
        ts = td - ta;
        if (ts < 1e-9)
            return xa;
        double tLocal = t - ta;
        a0 = pos0;
        a1 = vel0;
        a2 = acc0 / 2.0;
        a3 = (20.0 * (posf - pos0) - (8.0 * velf + 12.0 * vel0) * ts - (3.0 * acc0 - accf) * ts * ts) / (2.0 * ts * ts * ts);
        a4 = (30.0 * (pos0 - posf) + (14.0 * velf + 16.0 * vel0) * ts + (3.0 * acc0 - 2.0 * accf) * ts * ts) / (2.0 * ts * ts * ts * ts);
        a5 = (12.0 * (posf - pos0) - (6.0 * velf + 6.0 * vel0) * ts - (acc0 - accf) * ts * ts) / (2.0 * ts * ts * ts * ts * ts);
        return Eval5th(a0, a1, a2, a3, a4, a5, tLocal);
    }
    pos0 = xd;
    posf = xf;
    vel0 = vd;
    velf = 0;
    acc0 = 0;
    accf = 0;
    ts = tf - td;
    if (ts < 1e-9)
        return xf;
    double tLocal = t - td;
    a0 = pos0;
    a1 = vel0;
    a2 = acc0 / 2.0;
    a3 = (20.0 * (posf - pos0) - (8.0 * velf + 12.0 * vel0) * ts - (3.0 * acc0 - accf) * ts * ts) / (2.0 * ts * ts * ts);
    a4 = (30.0 * (pos0 - posf) + (14.0 * velf + 16.0 * vel0) * ts + (3.0 * acc0 - 2.0 * accf) * ts * ts) / (2.0 * ts * ts * ts * ts);
    a5 = (12.0 * (posf - pos0) - (6.0 * velf + 6.0 * vel0) * ts - (acc0 - accf) * ts * ts) / (2.0 * ts * ts * ts * ts * ts);
    return Eval5th(a0, a1, a2, a3, a4, a5, tLocal);
}

// Single fifth-order polynomial position profile from x0 to xf over
// total time T. Edge case T < 1e-9 returns xf, matching the reference.
inline double Single5thPosition(double x0, double xf, double T, double t)
{
    if (T < 1e-9)
        return xf;
    double tau = t / T;
    if (tau > 1.0)
        tau = 1.0;
    double s = 10.0 * tau * tau * tau - 15.0 * tau * tau * tau * tau + 6.0 * tau * tau * tau * tau * tau;
    return x0 + s * (xf - x0);
}

// Step count for sampling a profile: round(tf / h), clamped to
// [0, kTrajectoryMaxSteps]. Returns 0 when inputs are non-positive.
inline int ComputeStepCount(double tf, double h)
{
    if (h <= 0.0 || tf <= 0.0)
        return 0;
    int n = static_cast<int>(tf / h + 0.5);
    if (n <= 0)
        return 0;
    if (n > kTrajectoryMaxSteps)
        n = kTrajectoryMaxSteps;
    return n;
}

// Half-up rounding to long, matching the reference behaviour for
// negative values (rounds away from zero, not toward -inf).
inline long RoundToLong(double value)
{
    return static_cast<long>(value >= 0.0 ? value + 0.5 : value - 0.5);
}

}  // namespace motion
}  // namespace humanoid
