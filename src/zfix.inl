/*
 * Custom ZFix: because Allegro's zfix is not precise
 */


#ifndef ZFIX_INL
#define ZFIX_INL

inline ZLong toZLong(float val)
{
	return ZLong(val * 10000);
}
inline ZLong toZLong(double val)
{
	return ZLong(val * 10000);
}
inline ZLong toZLong(int32_t val)
{
	return ZLong(val * 10000);
}
inline zfix zslongToFix(ZLong val)
{
	zfix t;
	t.val = val;
	return t;
}
inline zfix floor(zfix fx)
{
	zfix t(fx);
	t.doFloor();
	return t;
}
inline zfix abs(zfix fx)
{
	zfix t(fx);
	t.doAbs();
	return t;
}

inline zfix operator +  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t += fx2;
	return t;
}
inline zfix operator +  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const int32_t v, const zfix fx)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const float v, const zfix fx)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t += v;
	return t;
}
inline zfix operator +  (const double v, const zfix fx)
{
	zfix t = fx.copy();
	t += v;
	return t;
}

inline zfix operator -  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t -= fx2;
	return t;
}
inline zfix operator -  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t -= v;
	return t;
}
inline zfix operator -  (const int32_t v, const zfix fx)
{
	zfix t(v);
	t -= fx;
	return t;
}
inline zfix operator -  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t -= v;
	return t;
}
inline zfix operator -  (const float v, const zfix fx)
{
	zfix t(v);
	t -= fx;
	return t;
}
inline zfix operator -  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t -= v;
	return t;
}
inline zfix operator -  (const double v, const zfix fx)
{
	zfix t(v);
	t -= fx;
	return t;
}

inline zfix operator *  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t *= fx2;
	return t;
}

inline zfix operator *  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const int32_t v, const zfix fx)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const float v, const zfix fx)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator *  (const double v, const zfix fx)
{
	zfix t = fx.copy();
	t *= v;
	return t;
}

inline zfix operator /  (const zfix fx, const zfix fx2)
{
	zfix t = fx.copy();
	t /= fx2;
	return t;
}

inline zfix operator /  (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t /= v;
	return t;
}

inline zfix operator /  (const int32_t v, const zfix fx)
{
	zfix t(v);
	t /= fx;
	return t;
}

inline zfix operator /  (const zfix fx, const float v)
{
	zfix t = fx.copy();
	t /= v;
	return t;
}

inline zfix operator /  (const float v, const zfix fx)
{
	zfix t(v);
	t /= fx;
	return t;
}

inline zfix operator /  (const zfix fx, const double v)
{
	zfix t = fx.copy();
	t /= v;
	return t;
}

inline zfix operator /  (const double v, const zfix fx)
{
	zfix t(v);
	t /= fx;
	return t;
}

inline zfix operator << (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t <<= v;
	return t;
}

inline zfix operator >> (const zfix fx, const int32_t v)
{
	zfix t = fx.copy();
	t >>= v;
	return t;
}

inline int32_t operator == (const zfix fx, const zfix fx2)
{
	return fx.val == fx2.val;
}
inline int32_t operator == (const zfix fx, const int32_t v)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const int32_t v, const zfix fx)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const zfix fx, const float v)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const float v, const zfix fx)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const zfix fx, const double v)
{
	return fx.val == toZLong(v);
}
inline int32_t operator == (const double v, const zfix fx)
{
	return fx.val == toZLong(v);
}

inline int32_t operator != (const zfix fx, const zfix fx2)
{
	return fx.val != fx2.val;
}
inline int32_t operator != (const zfix fx, const int32_t v)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const int32_t v, const zfix fx)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const zfix fx, const float v)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const float v, const zfix fx)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const zfix fx, const double v)
{
	return fx.val != toZLong(v);
}
inline int32_t operator != (const double v, const zfix fx)
{
	return fx.val != toZLong(v);
}

inline int32_t operator <  (const zfix fx, const zfix fx2)
{
	return fx.val < fx2.val;
}
inline int32_t operator <  (const zfix fx, const int32_t v)
{
	return fx.val < toZLong(v);
}
inline int32_t operator <  (const int32_t v, const zfix fx)
{
	return toZLong(v) < fx.val;
}
inline int32_t operator <  (const zfix fx, const float v)
{
	return fx.val < toZLong(v);
}
inline int32_t operator <  (const float v, const zfix fx)
{
	return toZLong(v) < fx.val;
}
inline int32_t operator <  (const zfix fx, const double v)
{
	return fx.val < toZLong(v);
}
inline int32_t operator <  (const double v, const zfix fx)
{
	return toZLong(v) < fx.val;
}

inline int32_t operator >  (const zfix fx, const zfix fx2)
{
	return fx.val > fx2.val;
}
inline int32_t operator >  (const zfix fx, const int32_t v)
{
	return fx.val > toZLong(v);
}
inline int32_t operator >  (const int32_t v, const zfix fx)
{
	return toZLong(v) > fx.val;
}
inline int32_t operator >  (const zfix fx, const float v)
{
	return fx.val > toZLong(v);
}
inline int32_t operator >  (const float v, const zfix fx)
{
	return toZLong(v) > fx.val;
}
inline int32_t operator >  (const zfix fx, const double v)
{
	return fx.val > toZLong(v);
}
inline int32_t operator >  (const double v, const zfix fx)
{
	return toZLong(v) > fx.val;
}

inline int32_t operator <=  (const zfix fx, const zfix fx2)
{
	return fx.val <= fx2.val;
}
inline int32_t operator <=  (const zfix fx, const int32_t v)
{
	return fx.val <= toZLong(v);
}
inline int32_t operator <=  (const int32_t v, const zfix fx)
{
	return toZLong(v) <= fx.val;
}
inline int32_t operator <=  (const zfix fx, const float v)
{
	return fx.val <= toZLong(v);
}
inline int32_t operator <=  (const float v, const zfix fx)
{
	return toZLong(v) <= fx.val;
}
inline int32_t operator <=  (const zfix fx, const double v)
{
	return fx.val <= toZLong(v);
}
inline int32_t operator <=  (const double v, const zfix fx)
{
	return toZLong(v) <= fx.val;
}

inline int32_t operator >=  (const zfix fx, const zfix fx2)
{
	return fx.val >= fx2.val;
}
inline int32_t operator >=  (const zfix fx, const int32_t v)
{
	return fx.val >= toZLong(v);
}
inline int32_t operator >=  (const int32_t v, const zfix fx)
{
	return toZLong(v) >= fx.val;
}
inline int32_t operator >=  (const zfix fx, const float v)
{
	return fx.val >= toZLong(v);
}
inline int32_t operator >=  (const float v, const zfix fx)
{
	return toZLong(v) >= fx.val;
}
inline int32_t operator >=  (const zfix fx, const double v)
{
	return fx.val >= toZLong(v);
}
inline int32_t operator >=  (const double v, const zfix fx)
{
	return toZLong(v) >= fx.val;
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


