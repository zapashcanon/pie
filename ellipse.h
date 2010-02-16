#ifndef __ELLIPSE_H__
#define __ELLIPSE_H__

#include <cairo.h>

void cairo_ellipse(cairo_t *c, double x, double y,
                               double rayon_x, double rayon_y,
                               double angle_start, double angle_stop);
void cairo_ellipse_negative(cairo_t *c, double x, double y,
                                        double rayon_x, double rayon_y,
                                        double angle_start, double angle_stop);

#endif /* __ELLIPSE_H__ */
