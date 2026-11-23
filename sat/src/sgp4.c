// sgp4.c - SGP4 near-Earth propagator (see sgp4.h)
#include "sat/sgp4.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define TWOPI (2.0 * M_PI)
#define X2O3 (2.0 / 3.0)

// WGS-72 constants (the ones TLEs are generated with)
#define MU 398600.8
#define RE 6378.135                       // km
#define XKE (60.0 / sqrt(RE * RE * RE / MU))
#define J2 0.001082616
#define J3 -0.00000253881
#define J4 -0.00000165597
#define J3OJ2 (J3 / J2)

const char *sgp4_strerror(int err) {
  switch (err) {
  case 0: return "ok";
  case 1: return "mean eccentricity out of range";
  case 2: return "mean motion <= 0";
  case 4: return "semi-latus rectum < 0";
  case 6: return "satellite has decayed";
  case 7: return "deep-space orbit (period >= 225 min) not supported";
  default: return "unknown error";
  }
}

int sgp4_init(sgp4_sat_t *s) {
  const double ecco = s->ecco, inclo = s->inclo, argpo = s->argpo;
  s->error = 0;

  // ---- initl: recover original mean motion (un-Kozai) and semi-major axis
  double eccsq = ecco * ecco;
  double omeosq = 1.0 - eccsq;
  double rteosq = sqrt(omeosq);
  double cosio = cos(inclo);
  double cosio2 = cosio * cosio;

  double ak = pow(XKE / s->no_kozai, X2O3);
  double d1 = 0.75 * J2 * (3.0 * cosio2 - 1.0) / (rteosq * omeosq);
  double del = d1 / (ak * ak);
  double adel = ak * (1.0 - del * del - del * (1.0 / 3.0 + 134.0 * del * del / 81.0));
  del = d1 / (adel * adel);
  s->no = s->no_kozai / (1.0 + del);

  double ao = pow(XKE / s->no, X2O3);
  double sinio = sin(inclo);
  double po = ao * omeosq;
  double con42 = 1.0 - 5.0 * cosio2;
  s->con41 = -con42 - cosio2 - cosio2;
  double posq = po * po;
  double rp = ao * (1.0 - ecco);
  s->ao = ao;

  if (TWOPI / s->no >= 225.0) {
    s->error = 7;
    return s->error;
  }
  if (s->no <= 0.0) {
    s->error = 2;
    return s->error;
  }

  // ---- sgp4init
  double ss = 78.0 / RE + 1.0;
  double qzms2t = pow((120.0 - 78.0) / RE, 4);

  s->isimp = (rp < (220.0 / RE + 1.0)) ? 1 : 0;

  double sfour = ss;
  double qzms24 = qzms2t;
  double perige = (rp - 1.0) * RE;
  if (perige < 156.0) {
    sfour = perige - 78.0;
    if (perige < 98.0)
      sfour = 20.0;
    qzms24 = pow((120.0 - sfour) / RE, 4);
    sfour = sfour / RE + 1.0;
  }

  double pinvsq = 1.0 / posq;
  double tsi = 1.0 / (ao - sfour);
  s->eta = ao * ecco * tsi;
  double etasq = s->eta * s->eta;
  double eeta = ecco * s->eta;
  double psisq = fabs(1.0 - etasq);
  double coef = qzms24 * pow(tsi, 4);
  double coef1 = coef / pow(psisq, 3.5);
  double cc2 = coef1 * s->no *
               (ao * (1.0 + 1.5 * etasq + eeta * (4.0 + etasq)) +
                0.375 * J2 * tsi / psisq * s->con41 *
                    (8.0 + 3.0 * etasq * (8.0 + etasq)));
  s->cc1 = s->bstar * cc2;
  double cc3 = 0.0;
  if (ecco > 1.0e-4)
    cc3 = -2.0 * coef * tsi * J3OJ2 * s->no * sinio / ecco;
  s->x1mth2 = 1.0 - cosio2;
  s->cc4 = 2.0 * s->no * coef1 * ao * omeosq *
           (s->eta * (2.0 + 0.5 * etasq) + ecco * (0.5 + 2.0 * etasq) -
            J2 * tsi / (ao * psisq) *
                (-3.0 * s->con41 * (1.0 - 2.0 * eeta + etasq * (1.5 - 0.5 * eeta)) +
                 0.75 * s->x1mth2 * (2.0 * etasq - eeta * (1.0 + etasq)) *
                     cos(2.0 * argpo)));
  s->cc5 = 2.0 * coef1 * ao * omeosq * (1.0 + 2.75 * (etasq + eeta) + eeta * etasq);

  double cosio4 = cosio2 * cosio2;
  double temp1 = 1.5 * J2 * pinvsq * s->no;
  double temp2 = 0.5 * temp1 * J2 * pinvsq;
  double temp3 = -0.46875 * J4 * pinvsq * pinvsq * s->no;
  s->mdot = s->no + 0.5 * temp1 * rteosq * s->con41 +
            0.0625 * temp2 * rteosq * (13.0 - 78.0 * cosio2 + 137.0 * cosio4);
  s->argpdot = -0.5 * temp1 * con42 +
               0.0625 * temp2 * (7.0 - 114.0 * cosio2 + 395.0 * cosio4) +
               temp3 * (3.0 - 36.0 * cosio2 + 49.0 * cosio4);
  double xhdot1 = -temp1 * cosio;
  s->nodedot = xhdot1 + (0.5 * temp2 * (4.0 - 19.0 * cosio2) +
                         2.0 * temp3 * (3.0 - 7.0 * cosio2)) * cosio;
  s->omgcof = s->bstar * cc3 * cos(argpo);
  s->xmcof = 0.0;
  if (ecco > 1.0e-4)
    s->xmcof = -X2O3 * coef * s->bstar / eeta;
  s->nodecf = 3.5 * omeosq * xhdot1 * s->cc1;
  s->t2cof = 1.5 * s->cc1;
  if (fabs(cosio + 1.0) > 1.5e-12)
    s->xlcof = -0.25 * J3OJ2 * sinio * (3.0 + 5.0 * cosio) / (1.0 + cosio);
  else
    s->xlcof = -0.25 * J3OJ2 * sinio * (3.0 + 5.0 * cosio) / 1.5e-12;
  s->aycof = -0.5 * J3OJ2 * sinio;
  s->delmo = pow(1.0 + s->eta * cos(s->mo), 3);
  s->sinmao = sin(s->mo);
  s->x7thm1 = 7.0 * cosio2 - 1.0;

  if (s->isimp != 1) {
    double cc1sq = s->cc1 * s->cc1;
    s->d2 = 4.0 * ao * tsi * cc1sq;
    double temp = s->d2 * tsi * s->cc1 / 3.0;
    s->d3 = (17.0 * ao + sfour) * temp;
    s->d4 = 0.5 * temp * ao * tsi * (221.0 * ao + 31.0 * sfour) * s->cc1;
    s->t3cof = s->d2 + 2.0 * cc1sq;
    s->t4cof = 0.25 * (3.0 * s->d3 + s->cc1 * (12.0 * s->d2 + 10.0 * cc1sq));
    s->t5cof = 0.2 * (3.0 * s->d4 + 12.0 * s->cc1 * s->d3 + 6.0 * s->d2 * s->d2 +
                      15.0 * cc1sq * (2.0 * s->d2 + cc1sq));
  }
  return 0;
}

int sgp4_propagate(sgp4_sat_t *s, double t, double r[3], double v[3]) {
  const double vkmpersec = RE * XKE / 60.0;
  s->error = 0;

  // ---- secular gravity and atmospheric drag
  double xmdf = s->mo + s->mdot * t;
  double argpdf = s->argpo + s->argpdot * t;
  double nodedf = s->nodeo + s->nodedot * t;
  double argpm = argpdf;
  double mm = xmdf;
  double t2 = t * t;
  double nodem = nodedf + s->nodecf * t2;
  double tempa = 1.0 - s->cc1 * t;
  double tempe = s->bstar * s->cc4 * t;
  double templ = s->t2cof * t2;

  if (s->isimp != 1) {
    double delomg = s->omgcof * t;
    double delm = s->xmcof * (pow(1.0 + s->eta * cos(xmdf), 3) - s->delmo);
    double temp = delomg + delm;
    mm = xmdf + temp;
    argpm = argpdf - temp;
    double t3 = t2 * t;
    double t4 = t3 * t;
    tempa = tempa - s->d2 * t2 - s->d3 * t3 - s->d4 * t4;
    tempe = tempe + s->bstar * s->cc5 * (sin(mm) - s->sinmao);
    templ = templ + s->t3cof * t3 + t4 * (s->t4cof + t * s->t5cof);
  }

  double nm = s->no;
  double em = s->ecco;
  double inclm = s->inclo;
  if (nm <= 0.0) {
    s->error = 2;
    return s->error;
  }
  double am = pow(XKE / nm, X2O3) * tempa * tempa;
  nm = XKE / pow(am, 1.5);
  em = em - tempe;
  if (em >= 1.0 || em < -0.001) {
    s->error = 1;
    return s->error;
  }
  if (em < 1.0e-6)
    em = 1.0e-6;
  mm = mm + s->no * templ;
  double xlm = mm + argpm + nodem;

  nodem = fmod(nodem, TWOPI);
  argpm = fmod(argpm, TWOPI);
  xlm = fmod(xlm, TWOPI);
  mm = fmod(xlm - argpm - nodem, TWOPI);

  double sinim = sin(inclm);
  double cosim = cos(inclm);

  // ---- long period periodics
  double ep = em, xincp = inclm, argpp = argpm, nodep = nodem, mp = mm;
  double sinip = sinim, cosip = cosim;

  double axnl = ep * cos(argpp);
  double temp = 1.0 / (am * (1.0 - ep * ep));
  double aynl = ep * sin(argpp) + temp * s->aycof;
  double xl = mp + argpp + nodep + temp * s->xlcof * axnl;

  // ---- solve Kepler's equation
  double u = fmod(xl - nodep, TWOPI);
  double eo1 = u;
  double tem5 = 9999.9;
  double sineo1 = 0.0, coseo1 = 0.0;
  int ktr = 1;
  while (fabs(tem5) >= 1.0e-12 && ktr <= 10) {
    sineo1 = sin(eo1);
    coseo1 = cos(eo1);
    tem5 = 1.0 - coseo1 * axnl - sineo1 * aynl;
    tem5 = (u - aynl * coseo1 + axnl * sineo1 - eo1) / tem5;
    if (fabs(tem5) >= 0.95)
      tem5 = tem5 > 0.0 ? 0.95 : -0.95;
    eo1 = eo1 + tem5;
    ktr++;
  }

  // ---- short period preliminary quantities
  double ecose = axnl * coseo1 + aynl * sineo1;
  double esine = axnl * sineo1 - aynl * coseo1;
  double el2 = axnl * axnl + aynl * aynl;
  double pl = am * (1.0 - el2);
  if (pl < 0.0) {
    s->error = 4;
    return s->error;
  }
  double rl = am * (1.0 - ecose);
  double rdotl = sqrt(am) * esine / rl;
  double rvdotl = sqrt(pl) / rl;
  double betal = sqrt(1.0 - el2);
  temp = esine / (1.0 + betal);
  double sinu = am / rl * (sineo1 - aynl - axnl * temp);
  double cosu = am / rl * (coseo1 - axnl + aynl * temp);
  double su = atan2(sinu, cosu);
  double sin2u = (cosu + cosu) * sinu;
  double cos2u = 1.0 - 2.0 * sinu * sinu;
  temp = 1.0 / pl;
  double temp1 = 0.5 * J2 * temp;
  double temp2 = temp1 * temp;

  // ---- update for short period periodics
  double mrt = rl * (1.0 - 1.5 * temp2 * betal * s->con41) +
               0.5 * temp1 * s->x1mth2 * cos2u;
  su = su - 0.25 * temp2 * s->x7thm1 * sin2u;
  double xnode = nodep + 1.5 * temp2 * cosip * sin2u;
  double xinc = xincp + 1.5 * temp2 * cosip * sinip * cos2u;
  double mvt = rdotl - nm * temp1 * s->x1mth2 * sin2u / XKE;
  double rvdot = rvdotl + nm * temp1 * (s->x1mth2 * cos2u + 1.5 * s->con41) / XKE;

  // ---- orientation vectors
  double sinsu = sin(su), cossu = cos(su);
  double snod = sin(xnode), cnod = cos(xnode);
  double sini = sin(xinc), cosi = cos(xinc);
  double xmx = -snod * cosi;
  double xmy = cnod * cosi;
  double ux = xmx * sinsu + cnod * cossu;
  double uy = xmy * sinsu + snod * cossu;
  double uz = sini * sinsu;
  double vx = xmx * cossu - cnod * sinsu;
  double vy = xmy * cossu - snod * sinsu;
  double vz = sini * cossu;

  r[0] = mrt * ux * RE;
  r[1] = mrt * uy * RE;
  r[2] = mrt * uz * RE;
  v[0] = (mvt * ux + rvdot * vx) * vkmpersec;
  v[1] = (mvt * uy + rvdot * vy) * vkmpersec;
  v[2] = (mvt * uz + rvdot * vz) * vkmpersec;

  if (mrt < 1.0) {
    s->error = 6;
    return s->error;
  }
  return 0;
}
