
#include <cmath>

class Complex {
	public:
		long double R, I;

		Complex() {
			R = 0.0l, I = 0.0l;
		}

		Complex(long double _r, long double _i) {
			R = _r, I = _i;
		}

		long double Modulus() {
			return sqrtl(R*R + I*I);
		}

		long double ModulusSqrd() {
			return R*R + I*I;
		}

		Complex operator + (Complex w) {
			return Complex(R + w.R, I + w.I);
		}

		Complex operator - (Complex w) {
			return Complex(R - w.R, I - w.I);
		}

		Complex operator * (Complex w) {
			return Complex(
				R * w.R - I * w.I,
				R * w.I + I * w.R
			);
		}

		Complex operator / (Complex w) {
			long double divInv = 1.0l / (w.R * w.R + w.I * w.I);

			return Complex(
				(R * w.R + I * w.I) * divInv,
				(I * w.R - R * w.I) * divInv
			);
		}

		Complex operator ^ (Complex w) {
			if (R == 0.0l && I == 0.0l) {
				return Complex(0, 0);
			}
			Complex exponent = w * Complex( logl(Modulus()), atan2l(I, R) );
			long double ex = expl(exponent.R);
			long double im = exponent.I;

			return Complex(
				ex * cosl(im),
				ex * sinl(im)
			);
		}
};