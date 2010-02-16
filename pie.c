#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <cairo.h>
#include <cairo-ps.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>

#include "ellipse.h"

struct color {
	double r;
	double g;
	double b;
	double a;
};

struct coord {
	double x;
	double y;
};

struct portion {
	struct color light;
	struct color dark;
	struct color dark_deg;
	struct color line;
	double line_width;

	struct coord t_cent;
	struct coord t_strt;
	struct coord t_stop;

	struct coord b_cent;
	struct coord b_strt;
	struct coord b_stop;

	double ang_strt;
	double ang_stop;

	double ca_strt;
	double ca_stop;

	char *legend;
	cairo_text_extents_t legend_exts;

	double tmp;
};

struct conf {
	char do_back;
	struct color back;

	double ratio;
	double decal;
	double height;
	double margin;

	double img_w;
	double img_h;

	double pie_w;
	double pie_h;

	double cx; /* centre x */
	double cy; /* centre y */

	double rx; /* rayon_x */
	double ry; /* rayon_y */

	double title_size;
	const char *title;
	struct color title_color;
	cairo_text_extents_t title_exts;;

	char draw_leg;
	double leg_size;
	struct color leg_color;
};



int     conf_imgx = 500;
int     conf_imgy = 500;
int     conf_margin = 0;
double  conf_ratio = 0.5;
double  conf_decal = 0.1;
double  conf_height = 0.4;

double  conf_line_width = 0.8f;
char   *conf_line_color = "#000000";

//char   *conf_back_color = "#DCDAD5";
char   *conf_back_color = "#ffffff";
char    conf_do_back = 1;

char   *conf_title = "My Wonderfull Graph";
double  conf_tit_siz = 25;
char   *conf_title_color = "#000000";

char    conf_leg = 0;
double  conf_leg_siz = 15;
char   *conf_leg_col = "#000000";

#define NB 12
int nb = NB;

int ent[NB] = {
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1
};

char *color[NB] = {
	"#ffbe00",
	"#ff0000",
	"#00ff00",
	"#0000ff",
	"#ffbe00",
	"#ff0000",
	"#00ff00",
	"#0000ff",
	"#ff0000",
	"#00ff00",
	"#0000ff",
	"#ff00ff"
};

double extract[NB] = {
	0,
	0,
	0,
	0,
	0,
	0.2,
	0.2,
	0,
	0,
	0,
	0,
	0
};

char *name[NB] = {
	"application a",
	"application b",
	"application c",
	"application d",
	"application e",
	"application f",
	"application g",
	"application h",
	"application i",
	"application j",
	"application k",
	"application l"
};





static inline
int hex_to_int(char c)
{
	if (c >= '0' && c <='9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c + 10 - 'A';
	else if (c >= 'a' && c <= 'f')
		return c + 10 - 'a';
	return 0;
}

static inline
void convert_rgba_hex(char *hex, unsigned char alpha, struct color *out)
{
	if (hex == NULL)
		return;

	if (hex[0] == '#')
		hex++;
	
	if (strlen(hex) != 6) {
		out->r = 0;
		out->g = 0;
		out->b = 0;
		out->a = 0;
		return;
	}
	
	out->r = ( hex_to_int(hex[0]) * 16.0f ) + hex_to_int(hex[1]);
	out->g = ( hex_to_int(hex[2]) * 16.0f ) + hex_to_int(hex[3]);
	out->b = ( hex_to_int(hex[4]) * 16.0f ) + hex_to_int(hex[5]);
	out->a = alpha;

	out->r /= 255.0f;
	out->g /= 255.0f;
	out->b /= 255.0f;
	out->a /= 255.0f;
}

static inline
void cairo_set_source_col(cairo_t *c, const struct color *col)
{
	cairo_set_source_rgba(c, col->r, col->g, col->b, col->a);
}

static inline col_dark(struct color *in, struct color *out, double dark) {
	out->r = in->r * dark;
	out->g = in->g * dark;
	out->b = in->b * dark;
	out->a = in->a;
}

static inline
double max(double *in, int nb)
{
	double ret = 0;
	int i;

	for (i=0; i<nb; i++)
		if (in[i] > ret)
			ret = in[i];
	return ret;
}

static inline
void draw_face_start(cairo_t *c, struct portion *p)
{
	/* face 1 */
	cairo_new_path(c);
	cairo_move_to(c, p->t_cent.x, p->t_cent.y);
	cairo_line_to(c, p->t_strt.x, p->t_strt.y);
	cairo_line_to(c, p->b_strt.x, p->b_strt.y);
	cairo_line_to(c, p->b_cent.x, p->b_cent.y);
	cairo_line_to(c, p->t_cent.x, p->t_cent.y);

	/* trace filled */
	cairo_set_line_width(c, 0.0); /* set line */
	cairo_set_source_col(c, &p->dark); /* set dark color */
	cairo_fill_preserve(c);

	/* trace line */
	cairo_set_line_width(c, p->line_width); /* set line */
	cairo_set_source_col(c, &p->line); /* set dark color */
	cairo_stroke(c);
}
	
static inline
void draw_face_stop(cairo_t *c, struct portion *p)
{
	/* face 2 */
	cairo_new_path(c);
	cairo_move_to(c, p->t_cent.x, p->t_cent.y);
	cairo_line_to(c, p->t_stop.x, p->t_stop.y);
	cairo_line_to(c, p->b_stop.x, p->b_stop.y);
	cairo_line_to(c, p->b_cent.x, p->b_cent.y);
	cairo_line_to(c, p->t_cent.x, p->t_cent.y);

	/* trace filled */
	cairo_set_line_width(c, 0.0); /* set line */
	cairo_set_source_col(c, &p->dark); /* set dark color */
	cairo_fill_preserve(c);

	/* trace line */
	cairo_set_line_width(c, p->line_width); /* set line */
	cairo_set_source_col(c, &p->line); /* set dark color */
	cairo_stroke(c);
}

static inline
void draw_face_rounded(cairo_t *c, struct conf *co, struct portion *p)
{
	cairo_pattern_t *pat;
	struct coord tstrt;
	struct coord bstop;
	double strt;
	double stop;

	/* set line */
	cairo_set_line_width(c, 0.0);
	
	/* set dark color */
	cairo_set_source_col(c, &p->dark);

	/* on arrange le start */
	if (p->ca_strt >= M_PI &&  p->ca_strt < 2.0 * M_PI) {
		strt = 2.0 * M_PI;
		tstrt.x = ( co->rx * cos(strt) ) + p->t_cent.x;
		tstrt.y = ( co->ry * sin(strt) ) + p->t_cent.y;
	}

	else {
		strt = p->ang_strt;
		tstrt.x = p->t_strt.x;
		tstrt.y = p->t_strt.y;
	}

	/* on arrange le stop */
	if (p->ca_stop > M_PI) {
		stop = M_PI;
		bstop.x = ( co->rx * cos(stop) ) + p->t_cent.x;
		bstop.y = ( co->rx * sin(stop) ) + p->t_cent.y + ( co->decal * co->ry );
	}

	else {
		stop = p->ang_stop;
		bstop.x = p->b_stop.x;
		bstop.y = p->b_stop.y;
	}

	/* face arrondie */
	cairo_new_path(c);
	cairo_move_to(c, tstrt.x, tstrt.y);
	cairo_ellipse(c, p->t_cent.x, p->t_cent.y, co->rx, co->ry, strt, stop);
	cairo_line_to(c, bstop.x, bstop.y);
	cairo_ellipse_negative(c, p->b_cent.x, p->b_cent.y, co->rx, co->ry, stop, strt);
	cairo_line_to(c, tstrt.x, tstrt.y);

	/* trace filled */
	cairo_set_line_width(c, 0.0);
	pat = cairo_pattern_create_linear (co->margin, 0.0, co->margin + co->pie_w, 256.0);
	cairo_pattern_add_color_stop_rgba (pat, 1, p->light.r, p->light.g, p->light.b, 1);
	cairo_pattern_add_color_stop_rgba (pat, 0, p->dark_deg.r, p->dark_deg.g, p->dark_deg.b, 1);
	cairo_set_source (c, pat);
	cairo_fill_preserve(c);

	/* trace line */
	cairo_set_line_width(c, p->line_width); /* set line */
	cairo_set_source_col(c, &p->line); /* set dark color */
	cairo_stroke(c);
}
	
static inline
void draw_face_top(cairo_t *c, struct conf *co, struct portion *p)
{
	/* Le toit */
	cairo_new_path(c);
	cairo_move_to(c, p->t_cent.x, p->t_cent.y);
	cairo_ellipse(c, p->t_cent.x, p->t_cent.y, co->rx, co->ry, p->ang_strt, p->ang_stop);
	cairo_line_to(c, p->t_cent.x, p->t_cent.y);

	/* trace filled */
	cairo_set_line_width(c, 0.0); /* set line */
	cairo_set_source_col(c, &p->light); /* set dark color */
	cairo_fill_preserve(c);

	/* trace line */
	cairo_set_source_col(c, &p->line); /* set dark color */
	cairo_set_line_width(c, p->line_width); /* set line */
	cairo_stroke(c);
}

/* on dessine en premier els piece qui se font ecraser
 *
 *  - start    partie gauche du haut vers le bas (a l'envers)
 *  - stop     partie droite du hat vers le bas (a l'endroit)
 *  - rounded  commence ou termine dans la partie basse
 */
static inline
void sort_start(struct portion *p, int pnb, struct portion **ps, int *psnb)
{
	int i;
	int j;
	struct portion *swap;

	/* extrait les valeurs */
	j = 0;
	for (i=0; i<pnb; i++)
		if (p[i].ca_strt > M_PI/2.0f && p[i].ca_strt < (3.0f*M_PI)/2.0f)
			ps[j++] = &p[i];
	*psnb = j;

	/* tri du plus grand vers le plus petit */
	for (i=0; i<*psnb; i++) {
		for (j=(*psnb)-1; j>i; j--) {
			if (ps[j]->ca_strt > ps[j-1]->ca_strt) {
				swap = ps[j];
				ps[j] = ps[j-1];
				ps[j-1] = swap;
			}
		}
	}
}

static inline
void sort_stop(struct portion *p, int pnb, struct portion **ps, int *psnb)
{
	int i;
	int j;
	struct portion *swap;

	/* extrait les valeurs */
	j = 0;
	for (i=0; i<pnb; i++) {
		if (p[i].ca_stop > (3.0f*M_PI)/2.0f && p[i].ca_stop <= 2.0f*M_PI) {
			p[i].tmp = p[i].ca_stop;
			ps[j++] = &p[i];
		}
			
		if (p[i].ca_stop >= 0.0f && p[i].ca_stop < M_PI/2.0f) {
			p[i].tmp = p[i].ca_stop + 2.0f*M_PI; /* update angle value for sort */
			ps[j++] = &p[i];
		}
	}
	*psnb = j;

	/* tri du plus grand vers le plus petit */
	for (i=0; i<*psnb; i++) {
		for (j=(*psnb)-1; j>i; j--) {
			if (ps[j]->tmp < ps[j-1]->tmp) {
				swap = ps[j];
				ps[j] = ps[j-1];
				ps[j-1] = swap;
			}
		}
	}
}

static inline
void sort_rounded(struct portion *p, int pnb, struct portion **ps, int *psnb)
{
	int i;
	int j;
	int inter;
	struct portion *swap;

	/* extrait rounded qui commence ou termine dans le bas */
	j = 0;
	for (i=0; i<nb; i++)
		if ( p[i].ca_stop > 0.0f && p[i].ca_stop <= M_PI/2.0f )
			ps[j++] = &p[i];
	inter = j;

	/* tri du plus petit vers le plus grand */
	for (i=0; i<inter; i++) {
		for (j=inter-1; j>i; j--) {
			if (ps[j]->ca_stop < ps[j-1]->ca_stop) {
				swap = ps[j];
				ps[j] = ps[j-1];
				ps[j-1] = swap;
			}
		}
	}

	/* extrait rounded qui commence ou termine dans le bas */
	j = inter;
	for (i=0; i<nb; i++)
		if (p[i].ca_strt >= M_PI/2.0f && p[i].ca_strt < M_PI )
			ps[j++] = &p[i];
	*psnb = j;

	/* tri du plus grand vers le plus petit */
	for (i=inter; i<*psnb; i++) {
		for (j=(*psnb)-1; j>i; j--) {
			if (ps[j]->ca_strt > ps[j-1]->ca_strt) {
				swap = ps[j];
				ps[j] = ps[j-1];
				ps[j-1] = swap;
			}
		}
	}
}

int trace(cairo_t *c, struct conf *co) {
	double y;
	double dec;
	double inc;
	int total = 0;
	double last = 0;
	double ang;
	int i;
	struct color col;
	struct color bg;
	struct portion *p;
	struct portion **ps;
	int psnb;
	double xdecal;
	double ydecal;
	double centerx;
	double centery;
	double hauteur;
	struct color colo;
	const char *eee = "Hello";
	double height_leg;
	double width_leg;
	struct coord a1;
	struct coord a2;

	/* build values total */
	total = 0.0f;
	for (i=0; i<nb; i++)
		total += (double)ent[i];

	/* memory used */
	p = malloc(sizeof(struct portion) * nb);
	ps = malloc(sizeof(struct portion *) * nb);

	/* title height */
	cairo_select_font_face (c, "Sans", CAIRO_FONT_SLANT_NORMAL,
	                        CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (c, co->title_size);
	cairo_text_extents (c, co->title, &co->title_exts);

	/* build legend mode 1 */
	if (co->draw_leg == 1) {
		cairo_select_font_face (c, "Sans", CAIRO_FONT_SLANT_NORMAL,
		                        CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (c, co->leg_size);
		dec = 0;
		for (i=0; i<nb; i++) {
			p[i].legend = name[i];
			cairo_text_extents (c, p[i].legend, &p[i].legend_exts);
			if (dec < p[i].legend_exts.height)
				dec = p[i].legend_exts.height;
		}
		height_leg = dec * (double)nb;
		width_leg = 0;
	}

	/* build legend mode 2 */
	else if (co->draw_leg == 2) {
		cairo_select_font_face (c, "Sans", CAIRO_FONT_SLANT_NORMAL,
		                        CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (c, co->leg_size);
		dec = 0;
		for (i=0; i<nb; i++) {
			p[i].legend = name[i];
			cairo_text_extents (c, p[i].legend, &p[i].legend_exts);
			if (dec < p[i].legend_exts.width)
				dec = p[i].legend_exts.width;
		}
		width_leg = dec;
		height_leg = 0;
	}

	/* no legend */
	else {
		height_leg = 0;
		width_leg = 0;
		dec = 0;
	}

	/* pie draw utility surface */
	co->pie_w  = co->img_w - ( ( 2.0 * co->margin ) + ( 2.0 * width_leg ) );
	co->pie_h  = co->img_h - ( ( 2.0 * co->margin ) + co->title_exts.height + 
	                         height_leg );

	/*               pie_w
	 * <-------------------------------->
	 *
	 *  rx
	 * <--->
	 *
	 * (R %  ex %  de %  de %  ex %  R %) * rx
	 * <---><----><----><----><----><--->
	 *
	 * pie_w = rx * 2 * ( ex + R + de )
	 * R = 1 (100%)
	 *
	 * pie_w = rx * 2 * ( ex + 1 + de )
	 * rx = pie_w / ( 2 * ( ex + de + 1 ) )
	 *
	 */

	/* pie radius */
	co->rx = co->pie_w / ( 2.0f * ( max(extract, nb) + co->decal + 1.0f ) );
	co->ry = co->rx * co->ratio;

	/* adjust radius */
	hauteur = ( co->ry * 2.0f ) + ( co->height * co->ry ) + 
	          ( max(extract, nb) * co->ry * 2.0f );
	if ( hauteur  > co->pie_h ) {
		co->rx *= (double)co->pie_h / hauteur;
		co->ry *= (double)co->pie_h / hauteur;
	}

	/* --- center y --- */

	/*
	 * top ----------------------
	 *      ^    ^  ^ 
	 *      |    |  | padding
	 *      |    |  v
	 *      |  c |  ^
	 *      |    |  | ex * ry
	 *      |    |  v
	 *      |    |  ^
	 *      |    v  | ry
	 *      |  - - -X 
	 * imgy |       | ry
	 *      |       v
	 *      |       ^
	 *      |       | ex * ry
	 *      |       v
	 *      |       ^
	 *      |       | height * ry
	 *      |       v
	 *      |       ^
	 *      |       | padding
	 *      v       v
	 * bottom -------------------     
	 *
	 *
	 * imgy = ( 2 * padding ) + ( 2 * ry ) + ( height * ry ) + ( 2 * ex * ry )
	 *
	 *           imgy - ( 2 * ry ) - ( height * ry ) - ( 2 * ex * ry )
	 * padding = -----------------------------------------------------
	 *                                   2
	 * 
	 * padding = c - ry - ( ex * ry )
	 *
	 *                        imgy - ( 2 * ry ) - ( height * ry ) - ( 2 * ex * ry )
	 * c - ry - ( ex * ry ) = -----------------------------------------------------
	 *                                               2
	 *
	 *     imgy - ( 2 * ry ) - ( height * ry ) - ( 2 * ex * ry )
	 * c = ----------------------------------------------------- + ry + ( ex * ry )
	 *                              2
	 *
	 *     imgy - ( height * ry )
	 * c = ----------------------
	 *               2
	 */

	co->cx = ( co->pie_w / 2.0f ) + co->margin + width_leg;
	co->cy = ( ( co->pie_h - ( co->height * co->ry ) ) / 2.0f ) +
	          co->margin + co->title_exts.height;

	/* build coordinates */
	last = 0;
	for (i=0; i<nb; i++) {

		/* start and stop angles */
		p[i].ang_strt = last;
		p[i].ang_stop = p[i].ang_strt + ( ( 2.0f * M_PI * 
		                (double)ent[i] ) / (double)total );
		last = p[i].ang_stop;

		/* angles for calc */
		p[i].ca_strt = p[i].ang_strt;
		p[i].ca_stop = p[i].ang_stop;

		while (p[i].ca_strt > 2.0 * M_PI)
			p[i].ca_strt -= 2.0 * M_PI;

		while (p[i].ca_strt < 0)
			p[i].ca_strt += 2.0 * M_PI;

		while (p[i].ca_stop > 2.0 * M_PI)
			p[i].ca_stop -= 2.0 * M_PI;

		while (p[i].ca_stop < 0)
			p[i].ca_stop += 2.0 * M_PI;

		/* on calcule le point central */
		p[i].t_cent.x = ( (co->decal + extract[i]) * co->rx * 
		                cos( (p[i].ang_strt+p[i].ang_stop)/2 ) ) + co->cx;
		p[i].t_cent.y = ( (co->decal + extract[i]) * co->ry *
		                sin( (p[i].ang_strt+p[i].ang_stop)/2 ) ) + co->cy;

		/* point de debut et fin de l'arc */
		p[i].t_strt.x = ( co->rx * cos(p[i].ang_strt) ) + p[i].t_cent.x;
		p[i].t_strt.y = ( co->ry * sin(p[i].ang_strt) ) + p[i].t_cent.y;
		p[i].t_stop.x = ( co->rx * cos(p[i].ang_stop) ) + p[i].t_cent.x;
		p[i].t_stop.y = ( co->ry * sin(p[i].ang_stop) ) + p[i].t_cent.y;

		/* Les point de l'autre bout */
		p[i].b_cent.x = p[i].t_cent.x;
		p[i].b_cent.y = p[i].t_cent.y + ( co->height * co->ry );
		p[i].b_strt.x = p[i].t_strt.x;
		p[i].b_strt.y = p[i].t_strt.y + ( co->height * co->ry );
		p[i].b_stop.x = p[i].t_stop.x;
		p[i].b_stop.y = p[i].t_stop.y + ( co->height * co->ry );

		/* colors */
		convert_rgba_hex(color[i], 0xff, &p[i].light);
		col_dark(&p[i].light, &p[i].dark_deg, 0.1);
		col_dark(&p[i].light, &p[i].dark, 0.5);
		convert_rgba_hex(conf_line_color, 0xff, &p[i].line);

		/* line witdh */
		p[i].line_width = conf_line_width;
	}

	/* draw backgroud */

	if (co->do_back) {
		cairo_new_path(c);
		cairo_rectangle(c, -1, -1, co->img_w+1, co->img_h+1);
		cairo_set_source_col(c, &co->back);
		cairo_fill(c);
		cairo_stroke(c);
	}

	/* draw title */
	if (co->title) {
		cairo_new_path(c);
		cairo_select_font_face (c, "Sans", CAIRO_FONT_SLANT_NORMAL,
		                        CAIRO_FONT_WEIGHT_NORMAL);

		cairo_set_font_size (c, co->title_size);
		cairo_set_source_col(c, &co->title_color);
		cairo_set_line_width(c, 6.0);
		cairo_move_to(c, (co->img_w / 2) - (co->title_exts.width / 2),
		                 co->title_exts.height);
		cairo_show_text(c, co->title);
		cairo_fill (c);
		cairo_stroke(c);
	}

	/* draw legend */
	if (co->draw_leg == 1) {

		cairo_select_font_face (c, "Sans", CAIRO_FONT_SLANT_NORMAL,
		                        CAIRO_FONT_WEIGHT_NORMAL);
		cairo_set_font_size (c, co->leg_size);

		y = co->margin + co->title_exts.height + co->pie_h;

#define DECFAC 0.15f

		a1.x = co->margin + ( dec * DECFAC );
		a1.y = dec * DECFAC;
		a2.x = co->margin + ( dec * (1.0f-DECFAC) );
		a2.y = dec * (1.0f-DECFAC);

		for (i=0; i<nb; i++) {

			cairo_new_path(c);
			cairo_move_to(c, a1.x, a1.y+y);
			cairo_line_to(c, a2.x, a1.y+y);
			cairo_line_to(c, a2.x, a2.y+y);
			cairo_line_to(c, a1.x, a2.y+y);
			cairo_line_to(c, a1.x, a1.y+y);
			//cairo_rectangle(c, a1.x, a1.y+y, a2.x, a2.y+y);

			/* fill */
			cairo_set_line_width(c, 0.0);
			cairo_set_source_col(c, &p[i].light);
			cairo_fill_preserve(c);

			/* trace line */
			cairo_set_line_width(c, 1.0f);
			cairo_set_source_rgba(c, 0,0,0,1);
			cairo_stroke(c);

			/* draw text */
			cairo_new_path(c);
			cairo_move_to(c, co->margin + dec + ( dec * 0.3f ),
			                 y - p[i].legend_exts.y_bearing);
			cairo_set_source_col(c, &co->leg_color);
			cairo_show_text(c, p[i].legend);
			cairo_fill (c);
			cairo_stroke(c);

			y += dec;
		}
	}

	/* draw pie */

	/* on ne dessine pas les faces cachées
	 *
	 *  - rounded  haut gauche et haut droit
	 *  - stop     bas gauce et haut gauche 
	 *  - start    haut droit et bas droit
	 */

	/* on dessine en premier els piece qui se font ecraser
	 *
	 *  - start    partie gauche du haut vers le bas (a l'envers)
	 *  - stop     partie droite du hat vers le bas (a l'endroit)
	 *  - rounded  commence ou termine dans la partie basse
	 */

	if (co->height > 0.0f) {

		sort_start(p, nb, ps, &psnb);
		for (i=0; i<psnb; i++)
			draw_face_start(c, ps[i]);

		sort_stop(p, nb, ps, &psnb);
		for (i=0; i<psnb; i++)
			draw_face_stop(c, ps[i]);

		sort_rounded(p, nb, ps, &psnb);
		for (i=0; i<psnb; i++)
			draw_face_rounded(c, co, ps[i]);

	}

	/*tous les tops */
	for (i=0; i<nb; i++)
		draw_face_top(c, co, &p[i]);
}

int main(void) {
	cairo_surface_t *s;
	cairo_t *c;
	struct conf co;

	/* copy conf */
	co.ratio      = conf_ratio;
	co.decal      = conf_decal;
	co.height     = conf_height;
	co.margin     = conf_margin;
	co.img_w      = conf_imgx;
	co.img_h      = conf_imgy;
	co.do_back    = conf_do_back;
	co.title      = conf_title;
	co.title_size = conf_tit_siz;
	co.draw_leg   = conf_leg;
	co.leg_size   = conf_leg_siz;
	convert_rgba_hex(conf_title_color, 0xff, &co.title_color);
	convert_rgba_hex(conf_back_color, 0xff, &co.back);
	convert_rgba_hex(conf_leg_col, 0xff, &co.leg_color);

	/* create image */
	s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, co.img_w, co.img_h);
//	s = cairo_ps_surface_create("amg.ps", co.img_w, co.img_h);
//	s = cairo_pdf_surface_create("amg.pdf", co.img_w, co.img_h);
//	s = cairo_svg_surface_create("amg.svg", co.img_w, co.img_h);

	/* create cairo */
	c = cairo_create(s);

	/* trace path */
	trace(c, &co);

	/* write image */
//	cairo_surface_write_to_png(s, "amg.png");
//	cairo_surface_finish(s);
//	cairo_surface_destroy(s);
//	cairo_destroy(c);
}
