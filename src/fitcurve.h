#ifndef APOLLO_FITCURVE_H
#define APOLLO_FITCURVE_H

/*
An Algorithm for Automatically Fitting Digitized Curves
by Philip J. Schneider
from "Graphics Gems", Academic Press, 1990
*/

#define TESTMODE

/*  fit_cubic.c	*/
/*	Piecewise cubic fitting code	*/

#ifndef GENERAL_H
#define GENERAL_H
#include "general.h"
#endif

#ifndef DRAW_H
#define DRAW_H
#include "draw.h"
#endif

#include <stdio.h>
#include <stdlib.h>

typedef Point2 *BezierCurve;

/* Forward declarations */
void		        FitCurve(Point2*, int, double, SDL_Surface*, Uint32, int);
static	void		FitCubic(Point2*, int, int, Vector2, Vector2, double, SDL_Surface*, Uint32, int);
static	double		*Reparameterize(Point2*, int, int, double*, BezierCurve);
static	double		NewtonRaphsonRootFind(BezierCurve, Point2, double);
static	Point2		BezierII(int, Point2*, double);
static	double 		B0(double), B1(double), B2(double), B3(double);
static	Vector2		ComputeLeftTangent(Point2*, int);
static	Vector2		ComputeRightTangent(Point2*, int);
static	Vector2		ComputeCenterTangent(Point2*, int);
static	double		ComputeMaxError(Point2*, int, int, BezierCurve, double*, int*);
static	double		*ChordLengthParameterize(Point2*, int, int);
static	BezierCurve	GenerateBezier(Point2*, int, int, double*, Vector2, Vector2);
static	Vector2		V2AddII(Vector2, Vector2);
static	Vector2		V2ScaleIII(Vector2, double);
static	Vector2		V2SubII(Vector2, Vector2);
static  double      V2SquaredLength(Vector2*);
static  double      V2DistanceBetween2Points(Point2*, Point2*);
static  Vector2     *V2Normalize(Vector2*);
static  double      V2Dot(Vector2*, Vector2*);
static  Vector2     *V2Scale(Vector2*, double);
static  Vector2     *V2Negate(Vector2*);
static  Vector2     *V2Add(Vector2*, Vector2*, Vector2*);

#define MAXPOINTS	1000		/* The most points you can have */

void DrawBezierCurve(int n, BezierCurve curve, SDL_Surface* sur, Uint32 color, int stroke) {
    float increment = 0.01;
    if (n == 3) {
        if (V2DistanceBetween2Points(&curve[0], &curve[1]) > V2DistanceBetween2Points(&curve[0], &curve[3]) * 2) {
            printf("Detected large distance, may result in weird curve. Reduced to line.\n");
            for (int i = 0; i < 4; ++i) {
                DrawPixel_Line(sur, curve[3], curve[0], stroke, color);
            }
            return;
        }

        // we have generated a cubic Bezier curve.
        for (float t = 0.0f; t <= 1; t += increment) {
            double x = pow(1-t, 3)*curve[0].x + 3*t*pow(1-t, 2)*curve[1].x + 3*t*t*(1-t)*curve[2].x + pow(t, 3)*curve[3].x;
            double y = pow(1-t, 3)*curve[0].y + 3*t*pow(1-t, 2)*curve[1].y + 3*t*t*(1-t)*curve[2].y + pow(t, 3)*curve[3].y;
            DrawPixel_CircleBrush(sur, {x, y}, stroke, color);
        }
    }

}


/*
 *  FitCurve :
 *  Fit a Bezier curve to a set of digitized points
 */
void FitCurve(Point2 *d, int nPts, double error, SDL_Surface* sur, Uint32 color, int stroke) {
//    Point2	*d;			/*  Array of digitized points	*/
//    int		nPts;		/*  Number of digitized points	*/
//    double	error;		/*  User-defined error squared	*/

    Vector2	tHat1, tHat2;	/*  Unit tangent vectors at endpoints */

    tHat1 = ComputeLeftTangent(d, 0);
    tHat2 = ComputeRightTangent(d, nPts - 1);
    FitCubic(d, 0, nPts - 1, tHat1, tHat2, error, sur, color, stroke);
}



/*
 *  FitCubic :
 *  	Fit a Bezier curve to a (sub)set of digitized points
 */
static void FitCubic(Point2* d, int first, int last, Vector2 tHat1, Vector2 tHat2, double error, SDL_Surface* sur, Uint32 color, int stroke) {
//    Point2	*d;			/*  Array of digitized points */
//    int		first, last;	/* Indices of first and last pts in region */
//    Vector2	tHat1, tHat2;	/* Unit tangent vectors at endpoints */
//    double	error;		/*  User-defined error squared	   */

    BezierCurve	bezCurve; /*Control points of fitted Bezier curve*/
    double	*u;		/*  Parameter values for point  */
    double	*uPrime;	/*  Improved parameter values */
    double	maxError;	/*  Maximum fitting error	 */
    int		splitPoint;	/*  Point to split point set at	 */
    int		nPts;		/*  Number of points in subset  */
    double	iterationError; /*Error below which you try iterating  */
    int		maxIterations = 4; /*  Max times to try iterating  */
    Vector2	tHatCenter;   	/* Unit tangent vector at splitPoint */
    int		i;

    iterationError = error * error;
    nPts = last - first + 1;

/*  Use heuristic if region only has two points in it */
    if (nPts == 2) {
        double dist = V2DistanceBetween2Points(&d[last], &d[first]) / 3.0;

        bezCurve = (Point2 *)malloc(4 * sizeof(Point2));
        bezCurve[0] = d[first];
        bezCurve[3] = d[last];
        V2Add(&bezCurve[0], V2Scale(&tHat1, dist), &bezCurve[1]);
        V2Add(&bezCurve[3], V2Scale(&tHat2, dist), &bezCurve[2]);
        DrawBezierCurve(3, bezCurve, sur, color, stroke);
        free((void *)bezCurve);
        return;
    }

    /*  Parameterize points, and attempt to fit curve */
    u = ChordLengthParameterize(d, first, last);
    bezCurve = GenerateBezier(d, first, last, u, tHat1, tHat2);

    /*  Find max deviation of points to fitted curve */
    maxError = ComputeMaxError(d, first, last, bezCurve, u, &splitPoint);
    if (maxError < error) {
        DrawBezierCurve(3, bezCurve, sur, color, stroke);
        free((void *)u);
        free((void *)bezCurve);
        return;
    }


    /*  If error not too large, try some reparameterization  */
    /*  and iteration */
    if (maxError < iterationError) {
        for (i = 0; i < maxIterations; i++) {
            uPrime = Reparameterize(d, first, last, u, bezCurve);
            free((void *)bezCurve);
            bezCurve = GenerateBezier(d, first, last, uPrime, tHat1, tHat2);
            maxError = ComputeMaxError(d, first, last,
                                       bezCurve, uPrime, &splitPoint);
            if (maxError < error) {
                DrawBezierCurve(3, bezCurve, sur, color, stroke);
                free((void *)u);
                free((void *)bezCurve);
                free((void *)uPrime);
                return;
            }
            free((void *)u);
            u = uPrime;
        }
    }

    /* Fitting failed -- split at max error point and fit recursively */
    free((void *)u);
    free((void *)bezCurve);
    tHatCenter = ComputeCenterTangent(d, splitPoint);
    FitCubic(d, first, splitPoint, tHat1, tHatCenter, error, sur, color, stroke);
    V2Negate(&tHatCenter);
    FitCubic(d, splitPoint, last, tHatCenter, tHat2, error, sur, color, stroke);
}


/*
 *  GenerateBezier :
 *  Use least-squares method to find Bezier control points for region.
 */
static BezierCurve GenerateBezier(Point2* d, int first, int last, double* uPrime, Vector2 tHat1, Vector2 tHat2) {
//    Point2	*d;			/*  Array of digitized points	*/
//    int		first, last;		/*  Indices defining region	*/
//    double	*uPrime;		/*  Parameter values for region */
//    Vector2	tHat1, tHat2;	/*  Unit tangents at endpoints	*/
    int 	i;
    Vector2 A[MAXPOINTS][2];	/* Precomputed rhs for eqn	*/
    int 	nPts;			/* Number of pts in sub-curve */
    double 	C[2][2];			/* Matrix C		*/
    double 	X[2];			/* Matrix X			*/
    double 	det_C0_C1,		/* Determinants of matrices	*/
            det_C0_X,
            det_X_C1;
    double 	alpha_l,		/* Alpha values, left and right	*/
            alpha_r;
    Vector2 	tmp;			/* Utility variable		*/
    BezierCurve	bezCurve;	/* RETURN bezier curve ctl pts	*/
    double      segLength;
    double      epsilon;

    bezCurve = (Point2 *)malloc(4 * sizeof(Point2));
    nPts = last - first + 1;


    /* Compute the A's	*/
    for (i = 0; i < nPts; i++) {
        Vector2		v1, v2;
        v1 = tHat1;
        v2 = tHat2;
        V2Scale(&v1, B1(uPrime[i]));
        V2Scale(&v2, B2(uPrime[i]));
        A[i][0] = v1;
        A[i][1] = v2;
    }

    /* Create the C and X matrices	*/
    C[0][0] = 0.0;
    C[0][1] = 0.0;
    C[1][0] = 0.0;
    C[1][1] = 0.0;
    X[0]    = 0.0;
    X[1]    = 0.0;

    for (i = 0; i < nPts; i++) {
        C[0][0] += V2Dot(&A[i][0], &A[i][0]);
        C[0][1] += V2Dot(&A[i][0], &A[i][1]);
        /*					C[1][0] += V2Dot(&A[i][0], &A[i][1]);*/
        C[1][0] = C[0][1];
        C[1][1] += V2Dot(&A[i][1], &A[i][1]);

        tmp = V2SubII(d[first + i],
                      V2AddII(
                              V2ScaleIII(d[first], B0(uPrime[i])),
                              V2AddII(
                                      V2ScaleIII(d[first], B1(uPrime[i])),
                                      V2AddII(
                                              V2ScaleIII(d[last], B2(uPrime[i])),
                                              V2ScaleIII(d[last], B3(uPrime[i]))))));


        X[0] += V2Dot(&A[i][0], &tmp);
        X[1] += V2Dot(&A[i][1], &tmp);
    }

    /* Compute the determinants of C and X	*/
    det_C0_C1 = C[0][0] * C[1][1] - C[1][0] * C[0][1];
    det_C0_X  = C[0][0] * X[1]    - C[1][0] * X[0];
    det_X_C1  = X[0]    * C[1][1] - X[1]    * C[0][1];

    /* Finally, derive alpha values	*/
    alpha_l = (det_C0_C1 == 0) ? 0.0 : det_X_C1 / det_C0_C1;
    alpha_r = (det_C0_C1 == 0) ? 0.0 : det_C0_X / det_C0_C1;

    /* If alpha negative, use the Wu/Barsky heuristic (see text) */
    /* (if alpha is 0, you get coincident control points that lead to
     * divide by zero in any subsequent NewtonRaphsonRootFind() call. */
    segLength = V2DistanceBetween2Points(&d[last], &d[first]);
    epsilon = 1.0e-6 * segLength;
    if (alpha_l < epsilon || alpha_r < epsilon) {
        /* fall back on standard (probably inaccurate) formula, and subdivide further if needed. */
        double dist = segLength / 3.0;
        bezCurve[0] = d[first];
        bezCurve[3] = d[last];
        V2Add(&bezCurve[0], V2Scale(&tHat1, dist), &bezCurve[1]);
        V2Add(&bezCurve[3], V2Scale(&tHat2, dist), &bezCurve[2]);
        return (bezCurve);
    }

    /*  First and last control points of the Bezier curve are */
    /*  positioned exactly at the first and last data points */
    /*  Control points 1 and 2 are positioned an alpha distance out */
    /*  on the tangent vectors, left and right, respectively */
    bezCurve[0] = d[first];
    bezCurve[3] = d[last];
    V2Add(&bezCurve[0], V2Scale(&tHat1, alpha_l), &bezCurve[1]);
    V2Add(&bezCurve[3], V2Scale(&tHat2, alpha_r), &bezCurve[2]);
    return (bezCurve);
}


/*
 *  Reparameterize:
 *	Given set of points and their parameterization, try to find
 *  a better parameterization.
 *
 */
static double *Reparameterize(Point2* d, int first, int last, double* u, BezierCurve bezCurve) {
//    Point2	*d;			/*  Array of digitized points	*/
//    int		first, last;		/*  Indices defining region	*/
//    double	*u;			/*  Current parameter values	*/
//    BezierCurve	bezCurve;	/*  Current fitted curve	*/

    int 	nPts = last-first+1;
    int 	i;
    double	*uPrime;		/*  New parameter values	*/

    uPrime = (double *)malloc(nPts * sizeof(double));
    for (i = first; i <= last; i++) {
        uPrime[i-first] = NewtonRaphsonRootFind(bezCurve, d[i], u[i-first]);
    }
    return (uPrime);
}



/*
 *  NewtonRaphsonRootFind :
 *	Use Newton-Raphson iteration to find better root.
 */
static double NewtonRaphsonRootFind(BezierCurve Q, Point2 P, double u) {
//    BezierCurve	Q;			/*  Current fitted curve	*/
//    Point2 		P;		/*  Digitized point		*/
//    double 		u;		/*  Parameter value for "P"	*/

    double 		numerator, denominator;
    Point2 		Q1[3], Q2[2];	/*  Q' and Q''			*/
    Point2		Q_u, Q1_u, Q2_u; /*u evaluated at Q, Q', & Q''	*/
    double 		uPrime;		/*  Improved u			*/
    int 		i;

    /* Compute Q(u)	*/
    Q_u = BezierII(3, Q, u);

    /* Generate control vertices for Q'	*/
    for (i = 0; i <= 2; i++) {
    Q1[i].x = (Q[i+1].x - Q[i].x) * 3.0;
    Q1[i].y = (Q[i+1].y - Q[i].y) * 3.0;
    }

    /* Generate control vertices for Q'' */
    for (i = 0; i <= 1; i++) {
    Q2[i].x = (Q1[i+1].x - Q1[i].x) * 2.0;
    Q2[i].y = (Q1[i+1].y - Q1[i].y) * 2.0;
    }

    /* Compute Q'(u) and Q''(u)	*/
    Q1_u = BezierII(2, Q1, u);
    Q2_u = BezierII(1, Q2, u);

    /* Compute f(u)/f'(u) */
    numerator = (Q_u.x - P.x) * (Q1_u.x) + (Q_u.y - P.y) * (Q1_u.y);
    denominator = (Q1_u.x) * (Q1_u.x) + (Q1_u.y) * (Q1_u.y) +
                  (Q_u.x - P.x) * (Q2_u.x) + (Q_u.y - P.y) * (Q2_u.y);
    if (denominator == 0.0f) return u;

    /* u = u - f(u)/f'(u) */
    uPrime = u - (numerator/denominator);
    return (uPrime);
}



/*
 *  Bezier :
 *  	Evaluate a Bezier curve at a particular parameter value
 *
 */
static Point2 BezierII(int degree, Point2* V, double t) {
//    int		degree;		/* The degree of the bezier curve	*/
//    Point2 	*V;		/* Array of control points		*/
//    double 	t;		/* Parametric value to find point for	*/

    int 	i, j;
    Point2 	Q;	        /* Point on curve at parameter t	*/
    Point2 	*Vtemp;		/* Local copy of control points		*/

    /* Copy array	*/
    Vtemp = (Point2 *)malloc((unsigned)((degree+1)
                                        * sizeof (Point2)));
    for (i = 0; i <= degree; i++) {
    Vtemp[i] = V[i];
    }

    /* Triangle computation	*/
    for (i = 1; i <= degree; i++) {
        for (j = 0; j <= degree-i; j++) {
            Vtemp[j].x = (1.0 - t) * Vtemp[j].x + t * Vtemp[j+1].x;
            Vtemp[j].y = (1.0 - t) * Vtemp[j].y + t * Vtemp[j+1].y;
        }
    }

    Q = Vtemp[0];
    free((void *)Vtemp);
    return Q;
}


/*
 *  B0, B1, B2, B3 :
 *	Bezier multipliers
 */
static double B0(double u) {
    double tmp = 1.0 - u;
    return (tmp * tmp * tmp);
}


static double B1(double u) {
    double tmp = 1.0 - u;
    return (3 * u * (tmp * tmp));
}

static double B2(double u)
{
    double tmp = 1.0 - u;
    return (3 * u * u * tmp);
}

static double B3(double u) {
    return (u * u * u);
}



/*
 * ComputeLeftTangent, ComputeRightTangent, ComputeCenterTangent :
 *Approximate unit tangents at endpoints and "center" of digitized curve
 */
static Vector2 ComputeLeftTangent(Point2* d, int end) {
//    Point2	*d;			/*  Digitized points*/
//    int		end;		/*  Index to "left" end of region */

    Vector2	tHat1;
    tHat1 = V2SubII(d[end+1], d[end]);
    tHat1 = *V2Normalize(&tHat1);
    return tHat1;
}

static Vector2 ComputeRightTangent(Point2* d, int end) {\
//    Point2	*d;			/*  Digitized points		*/
//    int		end;		/*  Index to "right" end of region */

    Vector2	tHat2;
    tHat2 = V2SubII(d[end-1], d[end]);
    tHat2 = *V2Normalize(&tHat2);
    return tHat2;
}


static Vector2 ComputeCenterTangent(Point2* d, int center) {
//    Point2	*d;			/*  Digitized points			*/
//    int		center;		/*  Index to point inside region	*/

    Vector2	V1, V2, tHatCenter;

    V1 = V2SubII(d[center-1], d[center]);
    V2 = V2SubII(d[center], d[center+1]);
    tHatCenter.x = (V1.x + V2.x)/2.0;
    tHatCenter.y = (V1.y + V2.y)/2.0;
    tHatCenter = *V2Normalize(&tHatCenter);
    return tHatCenter;
}


/*
 *  ChordLengthParameterize :
 *	Assign parameter values to digitized points
 *	using relative distances between points.
 */
static double *ChordLengthParameterize(Point2* d, int first, int last) {
//    Point2	*d;			/* Array of digitized points */
//    int		first, last;		/*  Indices defining region	*/

    int		i;
    double	*u;			/*  Parameterization		*/

    u = (double *)malloc((unsigned)(last-first+1) * sizeof(double));

    u[0] = 0.0;
    for (i = first+1; i <= last; i++) {
        u[i-first] = u[i-first-1] + V2DistanceBetween2Points(&d[i], &d[i-1]);
    }

    for (i = first + 1; i <= last; i++) {
        u[i-first] = u[i-first] / u[last-first];
    }

    return(u);
}




/*
 *  ComputeMaxError :
 *	Find the maximum squared distance of digitized points
 *	to fitted curve.
*/
static double ComputeMaxError(Point2* d, int first, int last, BezierCurve bezCurve, double* u, int* splitPoint) {
//    Point2	*d;			/*  Array of digitized points	*/
//    int		first, last;		/*  Indices defining region	*/
//    BezierCurve	bezCurve;		/*  Fitted Bezier curve		*/
//    double	*u;			/*  Parameterization of points	*/
//    int		*splitPoint;		/*  Point of maximum error	*/

    int		i;
    double	maxDist;		/*  Maximum error		*/
    double	dist;		/*  Current error		*/
    Point2	P;			/*  Point on curve		*/
    Vector2	v;			/*  Vector from point to curve	*/

    *splitPoint = (last - first + 1)/2;
    maxDist = 0.0;
    for (i = first + 1; i < last; i++) {
        P = BezierII(3, bezCurve, u[i-first]);
        v = V2SubII(P, d[i]);
        dist = V2SquaredLength(&v);
        if (dist >= maxDist) {
            maxDist = dist;
            *splitPoint = i;
        }
    }
    return (maxDist);
}

// TODO: Come back and replace with general.h functions.
static Vector2 V2AddII(Vector2 a, Vector2 b) {
    Vector2	c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return (c);
}

static Vector2 V2ScaleIII(Vector2 v, double s) {
    Vector2 result;
    result.x = v.x * s;
    result.y = v.y * s;
    return (result);
}

static Vector2 V2SubII(Vector2 a, Vector2 b) {
    Vector2	c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    return (c);
}

static double V2SquaredLength(Vector2* v) {
    return (v->x * v->x + v->y * v->y);
}

static double V2DistanceBetween2Points(Point2 * a, Point2* b) {
    return sqrt(pow(b->x - a->x, 2) + pow(b->y - a->y, 2));
}

static Vector2* V2Normalize(Vector2* v) {
    double dist = sqrt(V2SquaredLength(v));
    if (dist != 0.0) {
        v->x /= dist;
        v->y /= dist;
    }
    return v;
}

static double V2Dot(Vector2* a, Vector2* b) {
    return a->x * b->x + a->y * b->y;
}

static Vector2* V2Add(Vector2* a, Vector2* b, Vector2* c) {
    c->x = a->x+b->x;
    c->y = a->y+b->y;
    return(c);
}

static Vector2* V2Scale(Vector2* v, double s) {
    if (V2SquaredLength(v) != 0) {
        v->x *= s;
        v->y *= s;
    }
    return v;
}


static Vector2* V2Negate(Vector2 *v) {
    v->x *= -1;
    v->y *= -1;
    return v;
}

#endif //APOLLO_FITCURVE_H
