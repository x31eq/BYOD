#pragma once

#include <pch.h>

class TubeScreamerToneWDF
{
public:
    TubeScreamerToneWDF() = default;

    void prepare (double fs)
    {
        C5.prepare ((float) fs);
        C6.prepare ((float) fs);
        C7.prepare ((float) fs);

        Vplus_R9.setVoltage (VplusVal);
    }

    void setParams (float toneParam)
    {
        toneParam = toneParam * 2.0f - 1.0f;
        auto toneSkew = (float) chowdsp::signum::signum (toneParam) * std::pow (std::abs (toneParam), 0.35f);
        toneSkew = jlimit (0.01f, 0.99f, (toneSkew + 1.0f) * 0.5f);

        P2_high.setResistanceValue ((1.0f - toneSkew) * P2val);
        P2_low.setResistanceValue (toneSkew * P2val);
    }

    inline float processSample (float x)
    {
        Vin_R7.setVoltage (x);
        R.compute();
        return wdft::voltage<float> (P3);
    }

private:
    static constexpr float P2val = 20.0e3f;
    static constexpr float VplusVal = 4.5f;

    // Port A
    wdft::ResistiveVoltageSourceT<float> Vin_R7 { 1.0e3f };
    wdft::CapacitorT<float> C5 { 0.22e-6f };
    wdft::WDFParallelT<float, decltype (Vin_R7), decltype (C5)> P1 { Vin_R7, C5 };
    wdft::ResistiveVoltageSourceT<float> Vplus_R9 { 10.0e3f };
    wdft::WDFParallelT<float, decltype (Vplus_R9), decltype (P1)> P2 { Vin_R7, P1 };

    // Port B
    wdft::ResistorT<float> R8 { 220.0f };
    wdft::CapacitorT<float> C6 { 0.22e-6f };
    wdft::WDFSeriesT<float, decltype (R8), decltype (C6)> S1 { R8, C6 };

    // Port C
    wdft::ResistorT<float> P2_low { P2val * 0.5f };

    // Port D
    wdft::ResistorT<float> P2_high { P2val * 0.5f };

    // Port E
    wdft::ResistorT<float> R11 { 1.0e3f };

    // Port F
    wdft::CapacitorT<float> C7 { 1.0e-6f };
    wdft::ResistorT<float> R12 { 1.0e3f };
    wdft::WDFSeriesT<float, decltype (C7), decltype (R12)> S2 { C7, R12 };
    wdft::ResistorT<float> P3 { 100.0e3f };
    wdft::WDFSeriesT<float, decltype (P3), decltype (S2)> S3 { P3, S2 };

    struct ImpedanceCalc
    {
        template <typename RType>
        static void calcImpedance (RType& R)
        {
            constexpr float Ag = 100.0f; // op-amp gain
            constexpr float Ri = 1.0e9f; // op-amp input impedance
            constexpr float Ro = 1.0e-1f; // op-amp output impedance

            const auto [Ra, Rb, Rc, Rd, Re, Rf] = R.getPortImpedances();

            // This scattering matrix was derived using the R-Solver python script (https://github.com/jatinchowdhury18/R-Solver),
            // invoked with command: r_solver.py --out scratch/tube_screamer_tone_scatt.txt scratch/tube_screamer_tone.txt
            R.setSMatrixData ({ { (((Ag + 1) * Rb * Rc - Ra * Rb - ((Ag + 1) * Ra - (Ag + 1) * Rb - (Ag + 1) * Rc) * Rd - (Ra - Rb - Rc) * Re) * Rf * Ri - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra - Rb) * Rc + (Ra - Rb - Rc) * Rd) * Re) * Rf + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra - Rb) * Rc + (Ra - Rb - Rc) * Rd) * Re + ((Ra - Rb) * Rc + (Ra - Rb - Rc) * Rd) * Rf + (Ra * Rb - Rb * Rc + (Ra - Rb - Rc) * Rd + (Ra - Rb - Rc) * Re + (Ra - Rb - Rc) * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * ((Ra * Rc + Ra * Rd) * Re * Rf + ((Ag + 1) * Ra * Rd + Ra * Re) * Rf * Ri - ((Ra * Rc + Ra * Rd) * Re + (Ra * Rc + Ra * Rd) * Rf + (Ra * Rd + Ra * Re + Ra * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (Ra * Rd * Re * Rf + ((Ag + 1) * Ra * Rb + (Ag + 1) * Ra * Rd + Ra * Re) * Rf * Ri - (Ra * Rd * Re + Ra * Rd * Rf + (Ra * Rb + Ra * Rd + Ra * Re + Ra * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * ((Ag + 1) * Ra * Rb * Rf * Ri - Ra * Rc * Re * Rf + (Ra * Rc * Re + Ra * Rc * Rf - Ra * Rb * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * (Ra * Rb * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd) * Rf - (Ra * Rb * Rc + Ra * Rb * Ri + (Ra * Rb + Ra * Rc) * Rd) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (Ra * Rb * Rc + Ra * Rb * Ri + (Ra * Rb + Ra * Rc) * Rd) * Ro / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) },
                                { -2 * ((Rb * Rc + Rb * Rd) * Re * Rf + (Ag * Rb * Rc + (Ag + 1) * Rb * Rd + Rb * Re) * Rf * Ri - ((Rb * Rc + Rb * Rd) * Re + (Rb * Rc + Rb * Rd) * Rf + (Rb * Rd + Rb * Re + Rb * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -(((Ag + 1) * Rb * Rc + Ra * Rb - ((Ag + 1) * Ra - (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd - (Ra - Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb - Ra * Rc) * Rd - ((Ra - Rb) * Rc + (Ra - Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb - Ra * Rc) * Rd - ((Ra - Rb) * Rc + (Ra - Rb + Rc) * Rd) * Re - ((Ra - Rb) * Rc + (Ra - Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc - (Ra - Rb + Rc) * Rd - (Ra - Rb + Rc) * Re - (Ra - Rb + Rc) * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * ((Ag * Ra * Rb - (Ag + 1) * Rb * Rd - Rb * Re) * Rf * Ri - (Ra * Rb * Rd + Rb * Rd * Re) * Rf + (Ra * Rb * Rd + Rb * Rd * Re + Rb * Rd * Rf + (Rb * Rd + Rb * Re + Rb * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (((Ag + 1) * Ra * Rb + (Ag + 1) * Rb * Rc) * Rf * Ri + (Ra * Rb * Rc + Rb * Rc * Re) * Rf - (Ra * Rb * Rc + Rb * Rc * Re + Rb * Rc * Rf + (Ra * Rb + Rb * Rc) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * ((Ra * Rb + Rb * Rc) * Rf * Ri + (Ra * Rb * Rc + Ra * Rb * Rd) * Rf - (Ra * Rb * Rc + Ra * Rb * Rd + (Ra * Rb + Rb * Rc) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * (Ra * Rb * Rc + Ra * Rb * Rd + (Ra * Rb + Rb * Rc) * Ri) * Ro / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) },
                                { -2 * (Rc * Rd * Re * Rf + ((Ag + 1) * Rc * Rd + Rb * Rc + Rc * Re) * Rf * Ri - (Rc * Rd * Re + Rc * Rd * Rf + (Rb * Rc + Rc * Rd + Rc * Re + Rc * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (((Ag + 1) * Rc * Rd + Rc * Re) * Rf * Ri + (Ra * Rc * Rd + Rc * Rd * Re) * Rf - (Ra * Rc * Rd + Rc * Rd * Re + Rc * Rd * Rf + (Rc * Rd + Rc * Re + Rc * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -(((Ag + 1) * Rb * Rc - Ra * Rb - ((Ag + 1) * Ra + (Ag + 1) * Rb - (Ag + 1) * Rc) * Rd - (Ra + Rb - Rc) * Re) * Rf * Ri + (Ra * Rb * Rc - (Ra * Rb - Ra * Rc) * Rd + ((Ra + Rb) * Rc - (Ra + Rb - Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc - (Ra * Rb - Ra * Rc) * Rd + ((Ra + Rb) * Rc - (Ra + Rb - Rc) * Rd) * Re + ((Ra + Rb) * Rc - (Ra + Rb - Rc) * Rd) * Rf - (Ra * Rb - Rb * Rc + (Ra + Rb - Rc) * Rd + (Ra + Rb - Rc) * Re + (Ra + Rb - Rc) * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * ((Ag + 1) * Rb * Rc * Rf * Ri + (Ra * Rb * Rc + (Ra + Rb) * Rc * Re) * Rf - (Ra * Rb * Rc + (Ra + Rb) * Rc * Re + (Ra + Rb) * Rc * Rf + Rb * Rc * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (Ra * Rc * Rd * Rf - Rb * Rc * Rf * Ri - (Ra * Rc * Rd - Rb * Rc * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * (Ra * Rc * Rd - Rb * Rc * Ri) * Ro / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) },
                                { -2 * (Rc * Rd * Re * Rf + (Ag * Rc - Rb) * Rd * Rf * Ri - (Rc * Rd * Re + Rc * Rd * Rf - Rb * Rd * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (((Ag + 1) * Rc + Ra) * Rd * Rf * Ri + (Ra * Rc * Rd + Rc * Rd * Re) * Rf - (Ra * Rc * Rd + Rc * Rd * Re + Rc * Rd * Rf + (Ra + Rc) * Rd * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * ((Ag * Ra + (Ag + 1) * Rb) * Rd * Rf * Ri + (Ra * Rb * Rd + (Ra + Rb) * Rd * Re) * Rf - (Ra * Rb * Rd + (Ra + Rb) * Rd * Re + (Ra + Rb) * Rd * Rf + Rb * Rd * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), (((Ag + 1) * Rb * Rc + Ra * Rb - ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc - (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc - (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc - (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc - (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc - (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc - (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (Ra * Rc * Rd * Rf + (Ra + Rb + Rc) * Rd * Rf * Ri - (Ra * Rc * Rd + (Ra + Rb + Rc) * Rd * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * (Ra * Rc * Rd + (Ra + Rb + Rc) * Rd * Ri) * Ro / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) },
                                { -2 * ((Ag * Rc - Rb) * Re * Rf * Ri - (Rb * Rc + (Rb + Rc) * Rd) * Re * Rf + (Rb * Re * Ri + (Rb * Rc + (Rb + Rc) * Rd) * Re) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (((Ag + 1) * Rc + Ra) * Re * Rf * Ri + (Ra * Rc + Ra * Rd) * Re * Rf - ((Ra + Rc) * Re * Ri + (Ra * Rc + Ra * Rd) * Re) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (Ra * Rd * Re * Rf - (Ag * Ra + (Ag + 1) * Rb) * Re * Rf * Ri - (Ra * Rd * Re - Rb * Re * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * (Ra * Rc * Re * Rf + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Re * Rf * Ri - (Ra * Rc * Re + (Ra + Rb + Rc) * Re * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd - (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd - ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd - ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd - (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * ((Ra + Rb + Rc) * Re * Ri + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Ro / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) },
                                { 2 * ((Ag * Rb * Rc + Ag * Rc * Re + (Ag * Rb + Ag * Rc) * Rd) * Rf * Ri - (Rb * Rf * Ri + (Rb * Rc + (Rb + Rc) * Rd) * Rf) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * ((Ag * Ra * Rd - Ag * Rc * Re) * Rf * Ri - ((Ra + Rc) * Rf * Ri + (Ra * Rc + Ra * Rd) * Rf) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * ((Ag * Ra * Rb + Ag * Ra * Rd + (Ag * Ra + Ag * Rb) * Re) * Rf * Ri - (Ra * Rd * Rf - Rb * Rf * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), 2 * ((Ag * Ra * Rb + (Ag * Ra + Ag * Rb + Ag * Rc) * Re) * Rf * Ri + (Ra * Rc * Rf + (Ra + Rb + Rc) * Rf * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -2 * ((Ag * Rb * Rc + (Ag * Ra + Ag * Rb + Ag * Rc) * Rd) * Rf * Ri - ((Ra + Rb + Rc) * Rf * Ri + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro), -(((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re - ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re - (Ra + Rb + Rc) * Rf) * Ri) * Ro) / (((Ag + 1) * Rb * Rc + Ra * Rb + ((Ag + 1) * Ra + (Ag + 1) * Rb + (Ag + 1) * Rc) * Rd + (Ra + Rb + Rc) * Re) * Rf * Ri + (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re) * Rf - (Ra * Rb * Rc + (Ra * Rb + Ra * Rc) * Rd + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Re + ((Ra + Rb) * Rc + (Ra + Rb + Rc) * Rd) * Rf + (Ra * Rb + Rb * Rc + (Ra + Rb + Rc) * Rd + (Ra + Rb + Rc) * Re + (Ra + Rb + Rc) * Rf) * Ri) * Ro) } });
        }
    };

    using RType = wdft::RootRtypeAdaptor<float, ImpedanceCalc, decltype (P1), decltype (S1), decltype (P2_low), decltype (P2_high), decltype (R11), decltype (S3)>;
    RType R { std::tie (P1, S1, P2_low, P2_high, R11, S3) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TubeScreamerToneWDF)
};
