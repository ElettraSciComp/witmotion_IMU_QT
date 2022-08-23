# Acceleration bias calculation {#acceleration-bias}

## Disclaimer
The algorithm explained here is determined during the tests. It can be different from the original undocumented algorithm used by Witmotion in their own sensor control tool.

## Data layout
In [witmotion_config_packet](\ref witmotion_config_packet) the `setting` field uses inverse-byte data layout:
- `setting.raw[0]` is the **fine** tuning part of bias from 0.0 to -0.25, divided into 256 grades, `uint8_t`
- `setting.raw[1]` is the **rough** tuning part of bias, imposed as signed integer, 127 bidirectional grades, `int8_t`

## Calculation
The sensor built-in calibration coefficient is 4, so the original formula looks as the following:
\f{eqnarray*}{
    B &=& bias \\
    B_a^{rough} &=& trunc(4 \cdot B) \; \mbox{if} \left| B \right| < 127 \\
    \left| B_a^{rough} \right| &=& 127 \; \mbox{otherwise} \\
    B_a^{fine} &=& \frac{255}{0.25} \left( 0.25 - \left| bias \bmod 0.25 \right| \right) \mbox{if}\; bias < 0 \\
    &=& \frac{255}{0.25} \left| bias \bmod 0.25 \right| \; \mbox{otherwise}
\f}

Then the fractional parts of \f$ B_a^{rough} \f$ and \f$  B_a^{fine} \f$ should be discarded and the values exposed as _signed_ integers into `setting.raw` subfields.

