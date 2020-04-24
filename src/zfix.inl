/*
 * Custom ZFix: because Allegro's zfix is not precise
 */


#ifndef ZFIX_INL
#define ZFIX_INL

inline zslong floatToZLong(double val)
{
	return zslong(val * 10000);
}
inline zfix zslongToFix(zslong val)
{
	zfix t(val/10000L, abs(val)%10000);
	return t;
}
inline zfix floor(zfix fx)
{
	zfix t(fx);
	t.dpart = 0;
	return t;
}
inline zfix abs(zfix fx)
{
	zfix t(fx);
	t.ipart = abs(t.ipart);
	return t;
}

inline zfix operator +  (const zfix x, const zfix y)
{
	zfix t;
	t.ipart = x.ipart + y.ipart;
	t.dpart = x.dpart + y.dpart;
	t.update();
	return t;
}
inline zfix operator +  (const zfix x, const int y)
{
	zfix t;
	t.ipart = x.ipart + y;
	t.dpart = x.dpart;
	return t;
}
inline zfix operator +  (const int x, const zfix y)
{
	zfix t;
	t.ipart = y.ipart + x;
	t.dpart = y.dpart;
	return t;
}
inline zfix operator +  (const zfix x, const long y)
{
	zfix t;
	t.ipart = x.ipart + y;
	t.dpart = x.dpart;
	return t;
}
inline zfix operator +  (const long x, const zfix y)
{
	zfix t;
	t.ipart = y.ipart + x;
	t.dpart = y.dpart;
	return t;
}
inline zfix operator +  (const zfix x, const float y)
{
	zfix t(y);
	t.ipart += x.ipart;
	t.dpart += x.dpart;
	t.update();
	return t;
}
inline zfix operator +  (const float x, const zfix y)
{
	zfix t(x);
	t.ipart += y.ipart;
	t.dpart += y.dpart;
	t.update();
	return t;
}
inline zfix operator +  (const zfix x, const double y)
{
	zfix t(y);
	t.ipart += x.ipart;
	t.dpart += x.dpart;
	t.update();
	return t;
}
inline zfix operator +  (const double x, const zfix y)
{
	zfix t(x);
	t.ipart += y.ipart;
	t.dpart += y.dpart;
	t.update();
	return t;
}

inline zfix operator -  (const zfix x, const zfix y)
{
	zfix t;
	t.ipart = x.ipart - y.ipart;
	t.dpart = x.dpart - y.dpart;
	t.update();
	return t;
}
inline zfix operator -  (const zfix x, const int y)
{
	zfix t;
	t.ipart = x.ipart - y;
	t.dpart = x.dpart;
	return t;
}
inline zfix operator -  (const int x, const zfix y)
{
	zfix t;
	t.ipart = y.ipart - x;
	t.dpart = y.dpart;
	return t;
}
inline zfix operator -  (const zfix x, const long y)
{
	zfix t;
	t.ipart = x.ipart - y;
	t.dpart = x.dpart;
	return t;
}
inline zfix operator -  (const long x, const zfix y)
{
	zfix t;
	t.ipart = y.ipart - x;
	t.dpart = y.dpart;
	return t;
}
inline zfix operator -  (const zfix x, const float y)
{
	zfix t(y);
	t.ipart -= x.ipart;
	t.dpart -= x.dpart;
	t.update();
	return t;
}
inline zfix operator -  (const float x, const zfix y)
{
	zfix t(x);
	t.ipart -= y.ipart;
	t.dpart -= y.dpart;
	t.update();
	return t;
}
inline zfix operator -  (const zfix x, const double y)
{
	zfix t(y);
	t.ipart -= x.ipart;
	t.dpart -= x.dpart;
	t.update();
	return t;
}
inline zfix operator -  (const double x, const zfix y)
{
	zfix t(x);
	t.ipart -= y.ipart;
	t.dpart -= y.dpart;
	t.update();
	return t;
}

inline zfix operator *  (const zfix x, const zfix y)
{
	zfix t;
	t.dpart = ((x.dpart * y.dpart) / 10000) + (x.ipart * y.dpart) + (y.ipart * x.dpart);
	t.ipart = (x.ipart * y.ipart) + (t.dpart / 10000);
	t.dpart %= 10000;
	t.update();
	return t;
}

inline zfix operator *  (const zfix x, const int y)
{
	zfix t;
	t.dpart = (y * x.dpart);
	t.ipart = (x.ipart * y) + (t.dpart / 10000);
	t.dpart %= 10000;
	t.update();
	return t;
}

inline zfix operator *  (const int x, const zfix y)
{
	zfix t;
	t.dpart = (x * y.dpart);
	t.ipart = (y.ipart * x) + (t.dpart / 10000);
	t.dpart %= 10000;
	t.update();
	return t;
}

inline zfix operator *  (const zfix x, const long y)
{
	zfix t;
	t.dpart = (y * x.dpart);
	t.ipart = (x.ipart * y) + (t.dpart / 10000);
	t.dpart %= 10000;
	t.update();
	return t;
}

inline zfix operator *  (const long x, const zfix y)
{
	zfix t;
	t.dpart = (x * y.dpart);
	t.ipart = (y.ipart * x) + (t.dpart / 10000);
	t.dpart %= 10000;
	t.update();
	return t;
}

inline zfix operator *  (const zfix x, const float y)
{
	zfix t(y);
	return t * x;
}

inline zfix operator *  (const float x, const zfix y)
{
	zfix t(x);
	return t * y;
}

inline zfix operator *  (const zfix x, const double y)
{
	zfix t(y);
	return t * x;
}

inline zfix operator *  (const double x, const zfix y)
{
	zfix t(x);
	return t * y;
}

inline zfix operator /  (const zfix x, const zfix y)
{
	return zslongToFix((x.getZLong() * 10000) / y.getZLong());
}

inline zfix operator /  (const zfix x, const int y)
{
	return zslongToFix((x.getZLong()) / y);
}

inline zfix operator /  (const int x, const zfix y)
{
	return zslongToFix((x * 10000 * 10000) / y.getZLong());
}

inline zfix operator /  (const zfix x, const long y)
{
	return zslongToFix((x.getZLong()) / y);
}

inline zfix operator /  (const long x, const zfix y)
{
	return zslongToFix((x * 10000 * 10000) / y.getZLong());
}

inline zfix operator /  (const zfix x, const float y)
{
	return zslongToFix((x.getZLong() * 10000) / floatToZLong(y));
}

inline zfix operator /  (const float x, const zfix y)
{
	return zslongToFix((floatToZLong(x) * 10000) / y.getZLong());
}

inline zfix operator /  (const zfix x, const double y)
{
	return zslongToFix((x.getZLong() * 10000) / floatToZLong(y));
}

inline zfix operator /  (const double x, const zfix y)
{
	return zslongToFix((floatToZLong(x) * 10000) / y.getZLong());
}

inline zfix operator << (const zfix x, const int y)
{
	long val = x.getZLong();
	return zslongToFix(val << y);
}

inline zfix operator >> (const zfix x, const int y)
{
	long val = x.getZLong();
	return zslongToFix(val >> y);
}

inline int operator == (const zfix x, const zfix y)
{
	return x.ipart == y.ipart && x.dpart == y.dpart;
}
inline int operator == (const zfix x, const int y)
{
	return x.ipart == y && x.dpart == 0;
}
inline int operator == (const int x, const zfix y)
{
	return y.ipart == x && y.dpart == 0;
}
inline int operator == (const zfix x, const long y)
{
	return x.ipart == y && x.dpart == 0;
}
inline int operator == (const long x, const zfix y)
{
	return y.ipart == x && y.dpart == 0;
}
inline int operator == (const zfix x, const float y)
{
	zfix t(y);
	return x.ipart == t.ipart && x.dpart == t.dpart;
}
inline int operator == (const float x, const zfix y)
{
	zfix t(x);
	return y.ipart == t.ipart && y.dpart == t.dpart;
}
inline int operator == (const zfix x, const double y)
{
	zfix t(y);
	return x.ipart == t.ipart && x.dpart == t.dpart;
}
inline int operator == (const double x, const zfix y)
{
	zfix t(x);
	return y.ipart == t.ipart && y.dpart == t.dpart;
}

inline int operator != (const zfix x, const zfix y)
{
	return x.ipart != y.ipart || x.dpart != y.dpart;
}
inline int operator != (const zfix x, const int y)
{
	return x.ipart != y || x.dpart != 0;
}
inline int operator != (const int x, const zfix y)
{
	return y.ipart != x || y.dpart != 0;
}
inline int operator != (const zfix x, const long y)
{
	return x.ipart != y || x.dpart != 0;
}
inline int operator != (const long x, const zfix y)
{
	return y.ipart != x || y.dpart != 0;
}
inline int operator != (const zfix x, const float y)
{
	zfix t(y);
	return x.ipart != t.ipart || x.dpart != t.dpart;
}
inline int operator != (const float x, const zfix y)
{
	zfix t(x);
	return y.ipart != t.ipart || y.dpart != t.dpart;
}
inline int operator != (const zfix x, const double y)
{
	zfix t(y);
	return x.ipart != t.ipart || x.dpart != t.dpart;
}
inline int operator != (const double x, const zfix y)
{
	zfix t(x);
	return y.ipart != t.ipart || y.dpart != t.dpart;
}

inline int operator <  (const zfix x, const zfix y)
{
	return (x.ipart < y.ipart || (x.ipart == y.ipart && x.dpart < y.dpart));
}
inline int operator <  (const zfix x, const int y)
{
	return (x.ipart < y);
}
inline int operator <  (const int x, const zfix y)
{
	return (x < y.ipart);
}
inline int operator <  (const zfix x, const long y)
{
	return (x.ipart < y);
}
inline int operator <  (const long x, const zfix y)
{
	return (x < y.ipart);
}
inline int operator <  (const zfix x, const float y)
{
	zfix t(y);
	return (x.ipart < t.ipart || (x.ipart == t.ipart && x.dpart < t.dpart));
}
inline int operator <  (const float x, const zfix y)
{
	zfix t(x);
	return (t.ipart < y.ipart || (t.ipart == y.ipart && t.dpart < y.dpart));
}
inline int operator <  (const zfix x, const double y)
{
	zfix t(y);
	return (x.ipart < t.ipart || (x.ipart == t.ipart && x.dpart < t.dpart));
}
inline int operator <  (const double x, const zfix y)
{
	zfix t(x);
	return (t.ipart < y.ipart || (t.ipart == y.ipart && t.dpart < y.dpart));
}

inline int operator >  (const zfix x, const zfix y)
{
	return (x.ipart > y.ipart || (x.ipart == y.ipart && x.dpart > y.dpart));
}
inline int operator >  (const zfix x, const int y)
{
	return (x.ipart > y || (x.ipart == y && x.dpart > 0));
}
inline int operator >  (const int x, const zfix y)
{
	return (x > y.ipart);
}
inline int operator >  (const zfix x, const long y)
{
	return (x.ipart > y || (x.ipart == y && x.dpart > 0));
}
inline int operator >  (const long x, const zfix y)
{
	return (x > y.ipart);
}
inline int operator >  (const zfix x, const float y)
{
	zfix t(y);
	return (x.ipart > t.ipart || (x.ipart == t.ipart && x.dpart > t.dpart));
}
inline int operator >  (const float x, const zfix y)
{
	zfix t(x);
	return (t.ipart > y.ipart || (t.ipart == y.ipart && t.dpart > y.dpart));
}
inline int operator >  (const zfix x, const double y)
{
	zfix t(y);
	return (x.ipart > t.ipart || (x.ipart == t.ipart && x.dpart > t.dpart));
}
inline int operator >  (const double x, const zfix y)
{
	zfix t(x);
	return (t.ipart > y.ipart || (t.ipart == y.ipart && t.dpart > y.dpart));
}

inline int operator <=  (const zfix x, const zfix y)
{
	return (x.ipart < y.ipart || (x.ipart == y.ipart && x.dpart <= y.dpart));
}
inline int operator <=  (const zfix x, const int y)
{
	return (x.ipart < y || (x.ipart == y && x.dpart <= 0));
}
inline int operator <=  (const int x, const zfix y)
{
	return (x <= y.ipart);
}
inline int operator <=  (const zfix x, const long y)
{
	return (x.ipart < y || (x.ipart == y && x.dpart <= 0));
}
inline int operator <=  (const long x, const zfix y)
{
	return (x <= y.ipart);
}
inline int operator <=  (const zfix x, const float y)
{
	zfix t(y);
	return (x.ipart < t.ipart || (x.ipart == t.ipart && x.dpart <= t.dpart));
}
inline int operator <=  (const float x, const zfix y)
{
	zfix t(x);
	return (t.ipart < y.ipart || (t.ipart == y.ipart && t.dpart <= y.dpart));
}
inline int operator <=  (const zfix x, const double y)
{
	zfix t(y);
	return (x.ipart < t.ipart || (x.ipart == t.ipart && x.dpart <= t.dpart));
}
inline int operator <=  (const double x, const zfix y)
{
	zfix t(x);
	return (t.ipart < y.ipart || (t.ipart == y.ipart && t.dpart <= y.dpart));
}

inline int operator >=  (const zfix x, const zfix y)
{
	return (x.ipart > y.ipart || (x.ipart == y.ipart && x.dpart >= y.dpart));
}
inline int operator >=  (const zfix x, const int y)
{
	return (x.ipart >= y);
}
inline int operator >=  (const int x, const zfix y)
{
	return (x > y.ipart || (x == y.ipart && 0 >= y.dpart));
}
inline int operator >=  (const zfix x, const long y)
{
	return (x.ipart >= y);
}
inline int operator >=  (const long x, const zfix y)
{
	return (x > y.ipart || (x == y.ipart && 0 >= y.dpart));
}
inline int operator >=  (const zfix x, const float y)
{
	zfix t(y);
	return (x.ipart > t.ipart || (x.ipart == t.ipart && x.dpart >= t.dpart));
}
inline int operator >=  (const float x, const zfix y)
{
	zfix t(x);
	return (t.ipart > y.ipart || (t.ipart == y.ipart && t.dpart >= y.dpart));
}
inline int operator >=  (const zfix x, const double y)
{
	zfix t(y);
	return (x.ipart > t.ipart || (x.ipart == t.ipart && x.dpart >= t.dpart));
}
inline int operator >=  (const double x, const zfix y)
{
	zfix t(x);
	return (t.ipart > y.ipart || (t.ipart == y.ipart && t.dpart >= y.dpart));
}
/*
inline zfix sqrt(zfix x)		  { zfix t;  t.v = fixsqrt(x.v);		return t; }
inline zfix cos(zfix x)		   { zfix t;  t.v = fixcos(x.v);		 return t; }
inline zfix sin(zfix x)		   { zfix t;  t.v = fixsin(x.v);		 return t; }
inline zfix tan(zfix x)		   { zfix t;  t.v = fixtan(x.v);		 return t; }
inline zfix acos(zfix x)		  { zfix t;  t.v = fixacos(x.v);		return t; }
inline zfix asin(zfix x)		  { zfix t;  t.v = fixasin(x.v);		return t; }
inline zfix atan(zfix x)		  { zfix t;  t.v = fixatan(x.v);		return t; }
inline zfix atan2(zfix x, zfix y)  { zfix t;  t.v = fixatan2(x.v, y.v);  return t; }


inline void get_translation_matrix(MATRIX *m, zfix x, zfix y, zfix z)
{
   get_translation_matrix(m, x.v, y.v, z.v);
}


inline void get_scaling_matrix(MATRIX *m, zfix x, zfix y, zfix z)
{
   get_scaling_matrix(m, x.v, y.v, z.v);
}


inline void get_x_rotate_matrix(MATRIX *m, zfix r)
{
   get_x_rotate_matrix(m, r.v);
}


inline void get_y_rotate_matrix(MATRIX *m, zfix r)
{
   get_y_rotate_matrix(m, r.v);
}


inline void get_z_rotate_matrix(MATRIX *m, zfix r)
{
   get_z_rotate_matrix(m, r.v);
}


inline void get_rotation_matrix(MATRIX *m, zfix x, zfix y, zfix z)
{
   get_rotation_matrix(m, x.v, y.v, z.v);
}


inline void get_align_matrix(MATRIX *m, zfix xfront, zfix yfront, zfix zfront, zfix xup, zfix yup, zfix zup)
{
   get_align_matrix(m, xfront.v, yfront.v, zfront.v, xup.v, yup.v, zup.v);
}


inline void get_vector_rotation_matrix(MATRIX *m, zfix x, zfix y, zfix z, zfix a)
{
   get_vector_rotation_matrix(m, x.v, y.v, z.v, a.v);
}


inline void get_transformation_matrix(MATRIX *m, zfix scale, zfix xrot, zfix yrot, zfix zrot, zfix x, zfix y, zfix z)
{
   get_transformation_matrix(m, scale.v, xrot.v, yrot.v, zrot.v, x.v, y.v, z.v);
}


inline void get_camera_matrix(MATRIX *m, zfix x, zfix y, zfix z, zfix xfront, zfix yfront, zfix zfront, zfix xup, zfix yup, zfix zup, zfix fov, zfix aspect)
{
   get_camera_matrix(m, x.v, y.v, z.v, xfront.v, yfront.v, zfront.v, xup.v, yup.v, zup.v, fov.v, aspect.v);
}


inline void qtranslate_matrix(MATRIX *m, zfix x, zfix y, zfix z)
{
   qtranslate_matrix(m, x.v, y.v, z.v);
}


inline void qscale_matrix(MATRIX *m, zfix scale)
{
   qscale_matrix(m, scale.v);
}


inline zfix vector_length(zfix x, zfix y, zfix z)
{
   zfix t;
   t.v = vector_length(x.v, y.v, z.v);
   return t;
}


inline void normalize_vector(zfix *x, zfix *y, zfix *z)
{
   normalize_vector(&x->v, &y->v, &z->v);
}


inline void cross_product(zfix x1, zfix y_1, zfix z1, zfix x2, zfix y2, zfix z2, zfix *xout, zfix *yout, zfix *zout)
{
   cross_product(x1.v, y_1.v, z1.v, x2.v, y2.v, z2.v, &xout->v, &yout->v, &zout->v);
}


inline zfix dot_product(zfix x1, zfix y_1, zfix z1, zfix x2, zfix y2, zfix z2)
{
   zfix t;
   t.v = dot_product(x1.v, y_1.v, z1.v, x2.v, y2.v, z2.v);
   return t;
}


inline void apply_matrix(MATRIX *m, zfix x, zfix y, zfix z, zfix *xout, zfix *yout, zfix *zout)
{
   apply_matrix(m, x.v, y.v, z.v, &xout->v, &yout->v, &zout->v);
}


inline void persp_project(zfix x, zfix y, zfix z, zfix *xout, zfix *yout)
{
   persp_project(x.v, y.v, z.v, &xout->v, &yout->v);
}
*/
#endif		  /* ifndef ZFIX_INL */


