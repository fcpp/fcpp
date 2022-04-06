include math;
settings.tex = "pdflatex";

pair DIM  = (5,3);   // size of a plot
pair OFFS = (2,2.5); // spacing between plots
int  ROWS = 2;       // rows of plots per page
int  COLS = 3;       // columns of plots per page
real MAX_CROP = 1.1; // maximum cropping allowed (usually 1.3)
real LOG_LIN = 2;    // factor of comparison between linear and logarithmic plots
bool LEGENDA = true; // whether to draw the legenda or not
bool SUBPLOT = false;// whether to compile subplots

// line styles and colors
pen[] styles = {solid+1, linetype(new real[] {10,5})+1, linetype(new real[] {6,3,0.5,3})+1, linetype(new real[] {1,2.5})+1};
pen[] colors = {mediumred, blue, heavygreen, heavymagenta, black, orange};

// draws a point with a given style
void pointer(pair p, int style, pen c) {
    real size = 0.07;
    if (style == 0) {
        draw(p-(size,size) -- p+(size,size), c);
        draw(p-(size,-size) -- p+(size,-size), c);
    }
    if (style == 1) {
        draw(p-(0,size) -- p+(0,size), c);
        draw(p-(size,0) -- p+(size,0), c);
    }
    if (style == 2) {
        draw(circle(p, size), c);
    }
    if (style == 3) {
        draw(p-(size,size) -- p-(size,-size) -- p+(size,size) -- p+(size,-size) -- cycle, c);
    }
    if (style == 4) {
        for (int i=0; i<360; i+=30) draw(p -- p+size*dir(i), c);
    }
}

// draws the legenda
void legenda(pair p, string[] names) {
    real step = 0.3;
    for (int i=0; i<names.length; ++i) {
        pen col = colors[i%colors.length];
        pen stl = styles[i%styles.length];
        draw(p+(0,i*step) -- p+(4*step,i*step), col + stl);
        pointer(p+(0,i*step), i%5, col);
        label(scale(0.7)*names[i], p+(5*step,i*step), align=right, col);
    }
    draw(p+(-step,-step) -- p + (-step,names.length*step) -- p + (5*step + 2.3,names.length*step) -- p+(5*step + 2.3,-step) -- cycle, black);
}

string format_number(real x, real s) {
    if (x == 0) return "$0$";
    if (0.01 <= s && s < 10000) return "$" + string(x) + "$";
    real e = floor(log10(max(s,realMin)));
    real l = pow10(e);
    return "$" + string(x / pow10(e)) + "\!\cdot\!10^{" + string(e) + "}$";
}

// approximates a real to the closest number equal to 1, 2 or 5 times a power of 10
real approx(real a) {
    real l = pow10(floor(log10(max(a,realMin))));
    if (a > 7.5 * l) return 10 * l;
    if (a > 3.5 * l) return 5 * l;
    if (a > 1.5 * l) return 2 * l;
    return l;
}

// approximate Y scale
real approx(real a, real b, real d) {
    return approx(0.6*(b - a)/d);
}

bool isfinite(real x) {
    return !(isnan(x) || x == inf || x == -inf);
}

// convert point coordinates into screen coordinates
real fit(real x, real a = 0, real b = 1, real l = 1, bool logmode = false) {
    return l*((logmode ? log10(x) : x)-a)/(b-a);
}

// produces a single plot
picture plot(real endx = 0, string ppath, string title, string xlabel, string ylabel, string[] names = new string[] {}, pair[][] values) {
    picture pic;
    unitsize(pic, 1cm);

    // scan points for min-max x and y values
    real bminx = inf, bminy = inf, bmaxx = -inf, bmaxy = -inf;
    real[] valy = {}, sumy = {0}, sqsumy = {0};
    bool logmode = false;
    for (int i=0; i<values.length; ++i) {
        real tmp = -inf;
        for (int j=0; j<values[i].length; ++j) if (isfinite(values[i][j].y)) {
            bminx = min(bminx, values[i][j].x);
            bmaxx = max(bmaxx, values[i][j].x);
            valy.push(values[i][j].y);
        }
    }
    if (bmaxx < bminx) {
        bminx = bminy = 0;
        bmaxx = bmaxy = 1;
    }
    if (bminx == bmaxx) {
        bminx = bminx - 0.5;
        bmaxx = bmaxx + 0.5;
    }

    if (valy.length > 0) {
        // scan y values for best area covered using linear plot
        int iminy = 0, imaxy = valy.length-1;
        valy = sort(valy);
        for (int i=iminy; i<=imaxy; ++i) {
            sumy[i+1] = sumy[i] + valy[i];
            sqsumy[i+1] = sqsumy[i] + valy[i]^2;
        }
        real area(int i, int j) {
            // mean{ -1 if cropped else 4(dist from top)(dist from bottom) }
            real ddsum = ((sumy[j+1] - sumy[i])*(valy[j] + valy[i]) - (sqsumy[j+1] - sqsumy[i]) - (j+1-i)*valy[i]*valy[j]);
            return valy[i] == valy[j] ? 0 : (4*ddsum / (valy[j] - valy[i])^2 + j+1-i) / valy.length - 1;
        }
        for (int l=valy.length-1; MAX_CROP*l >= valy.length; --l)
            for (int i=0; i+l < valy.length; ++i)
                if (area(iminy, imaxy) < area(i, i+l)) {
                    iminy = i;
                    imaxy = i+l;
                }
        for (int l=valy.length-1; MAX_CROP*l >= valy.length; --l)
            for (int i=0; i+l < valy.length; ++i)
                if (0.9 * area(iminy, imaxy) < area(i, i+l)) {
                    iminy = i;
                    imaxy = i+l;
                    i = valy.length;
                    l = 0;
                }
        real besta = area(iminy, imaxy);
        // scan y values for best area covered using logarithmic plot
        int lminy = 0, lmaxy = valy.length-1;
        real[] lvaly = copy(valy);
        for (int i=0; i<=lmaxy; ++i)
            if (valy[i] <= 0) {
                valy[i] = nan;
                ++lminy;
            }
            else valy[i] = log10(valy[i]);
        sumy[lminy] = sqsumy[lminy] = 0;
        for (int i=lminy; i<=lmaxy; ++i) {
            sumy[i+1] = sumy[i] + valy[i];
            sqsumy[i+1] = sqsumy[i] + valy[i]^2;
        }
        for (int l=valy.length-1; MAX_CROP*l >= valy.length; --l)
            for (int i=0; i+l < valy.length; ++i)
                if (!isnan(valy[i]) && area(lminy, lmaxy) < area(i, i+l)) {
                    lminy = i;
                    lmaxy = i+l;
                }
        for (int l=valy.length-1; MAX_CROP*l >= valy.length; --l)
            for (int i=0; i+l < valy.length; ++i)
                if (!isnan(valy[i]) && 0.9 * area(lminy, lmaxy) < area(i, i+l)) {
                    lminy = i;
                    lmaxy = i+l;
                    i = valy.length;
                    l = 0;
                }
    // compare and choose the final plot window
        logmode = area(lminy, lmaxy) > besta * LOG_LIN;
        if (MAX_CROP == 0) {
            bminy = lvaly[0]*0.999;
            bmaxy = lvaly[lvaly.length-1]*1.001;
        } else if (logmode) {
            bminy = valy[lminy];
            bmaxy = valy[lmaxy];
        } else {
            bminy = lvaly[iminy];
            bmaxy = lvaly[imaxy];
        }
        real diff = (bmaxy-bminy)/100;
        bminy -= diff;
        bmaxy += diff;
        write(title + " linear: " + string(round(10000*besta)/100) + "% log: " + string(round(10000*area(lminy, lmaxy))/100) + "% (I choose " + (logmode ? "log)" : "lin)"));
    }
    if (bminy == bmaxy) {
        bminy = bminy - 0.5;
        bmaxy = bmaxy + 0.5;
    }
    real xscale = approx(bminx, bmaxx, DIM.x);
    real yscale = approx(bminy, bmaxy, DIM.y);

    for (int l=0; l<values.length; ++l) {
        pen pn = styles[l%styles.length]+colors[l%colors.length];
        if (values[l].length == 0) continue;
        path p;
        bool drawing = false;
        pair lp = (0,nan);
        for (int i=0; i<values[l].length; ++i) {
            real x = fit(values[l][i].x, bminx, bmaxx, DIM.x);
            real y = fit(values[l][i].y, bminy, bmaxy, DIM.y, logmode);
            if (isnan(lp.y)) lp = (x,y);
            else if (lp.y == -inf) lp = (x,0);
            else if (lp.y == inf) lp = (x,DIM.y);
            else if (isnan(y)) lp = lp;
            else if (y == -inf) lp = (lp.x,0);
            else if (y == inf) lp = (lp.x,DIM.y);
            else if (y != lp.y) {
                pair p0 = (lp.x + (x-lp.x)*(0    -lp.y)/(y-lp.y), 0);
                pair p1 = (lp.x + (x-lp.x)*(DIM.y-lp.y)/(y-lp.y), DIM.y);
                if      (lp.y < 0     && y > DIM.y) draw(pic, p0 -- p1, pn);
                else if (lp.y > DIM.y && y < 0    ) draw(pic, p1 -- p0, pn);
                else if ((y < 0)     ^ (lp.y < 0    )) lp = p0;
                else if ((y > DIM.y) ^ (lp.y > DIM.y)) lp = p1;
            }
            if (0 <= y && y <= DIM.y) {
                if (!drawing) p = lp;
                p = p -- (x,y);
                drawing = true;
            } else {
                if (drawing) draw(pic, p -- lp, pn);
                drawing = false;
            };
            lp = (x,y);
        }
        if (drawing) draw(pic, p, pn);
    }
    draw(pic, (0,0) -- (0,DIM.y+0.8), black, EndArrow(4));
    draw(pic, (0,0) -- (DIM.x+1.0,0), black, EndArrow(4));
    if (endx != 0) {
        endx = fit(endx, bminx, bmaxx, DIM.x);
        draw(pic, (endx,0) -- (endx,DIM.y), black+0.5);
    }
    if (length(xlabel) > 0) {
        real offs = length(xlabel) < 3 ? 0.1 : length(xlabel) < 6 ? 0 : -0.1;
        label(pic, scale(0.5)*("\textit{"+xlabel+"}"), (DIM.x*(0.5*xscale/(bmaxx-bminx) + 1) + offs,-0.15), align=E);
    }
    if (length(ylabel) > 0) {
        real offs = length(ylabel) < 3 ? 0.1 : length(ylabel) < 6 ? 0 : -0.1;
        label(pic, rotate(90)*scale(0.5)*("\textit{"+ylabel+"}"), (-0.15,DIM.y+offs), align=N);
    }
    if (length(title) > 0)
        label(pic, scale(0.6)*replace(title, "%", "\%"), (DIM.x/2+0.55,DIM.y+0.7));

    Label adapt_label(string text, real maxscale, real maxlength, pair align = (0,0)) {
        picture pp;
        unitsize(pp, 1cm);
        label(pp, scale(maxscale)*text, (0,0));
        real len = size(pp, true).x;
        if (len > maxlength) maxscale *= maxlength / len;
        if (align != E) text = "\phantom{pd}" + text;
        if (align != W) text = text + "\phantom{pd}";
        return scale(maxscale) * text;
    }

    int xta = ceil( bminx/xscale);
    int xtb = floor(bmaxx/xscale);
    for (int x=xta; x <= xtb; ++x) {
        real rxs = x*xscale;
        real rx = DIM.x*(rxs-bminx)/(bmaxx-bminx);
        draw(pic, (rx,0) -- (rx,-.1));
        string s = format_number(rxs, max(xtb, -xta)*xscale);
        real ml = DIM.x*xscale/(bmaxx-bminx);
        label(pic, adapt_label(s, 0.4,  ml), (rx,-.2));
    }
    int yta = ceil( bminy/yscale);
    int ytb = floor(bmaxy/yscale);
    for (int y=yta; y <= ytb; ++y) {
        real rys = y*yscale;
        real ry = fit(rys, bminy, bmaxy, DIM.y);
        draw(pic, (0,ry) -- (-.1,ry));
        string s = logmode ? ("$10^{"+string(rys)+"}$") : format_number(rys, max(ytb, -yta)*yscale);
        label(pic, scale(0.4)*s, (0,ry), align=W);
    }
    int common_suffix = rfind(names[0], " ");
    string append_suffix = "";
    if (common_suffix > 0) {
        string suffix = substr(names[0], common_suffix);
        common_suffix = length(names[0]) - common_suffix;
        for (int i=1; i<names.length; ++i)
            if (substr(names[i], length(names[i])-common_suffix) != suffix) {
                common_suffix = 0;
                break;
            }
    }
    if (common_suffix == 0) {
        common_suffix = rfind(names[0], "-");
        append_suffix = ")";
        if (common_suffix > 0) {
            string suffix = substr(names[0], common_suffix);
            common_suffix = length(names[0]) - common_suffix;
            for (int i=1; i<names.length; ++i)
                if (substr(names[i], length(names[i])-common_suffix) != suffix) {
                    common_suffix = 0;
                    break;
                }
        }
    }
    for (int i=0; i<names.length; ++i) {
        if (common_suffix > 0) {
            names[i] = substr(names[i], 0, length(names[i])-common_suffix) + append_suffix;
        }
        for (int j=0; j<10; ++j)
            names[i] = replace(names[i], "<"+string(j)+">", " "+string(j));
        names[i] = replace(replace(names[i], "<", " ("), ">", ")");
        if (find(names[i], "  ") >= 0)
            names[i] = replace(names[i], "  ", " (") + ")";
    }
    if (LEGENDA) {
        for (int i=0; i<names.length; i+=2) {
            draw(pic, (0,-0.5-i/8) -- (0.8,-0.5-i/8), styles[i%styles.length]+colors[i%colors.length]);
            label(pic, adapt_label(names[i], 0.5, DIM.x/2 - 0.5, E), (0.9,-0.5-i/8), align=E);
            if (i+1 < names.length) {
                draw(pic, (DIM.x/2+0.5,-0.5-i/8) -- (DIM.x/2+1.3,-0.5-i/8), styles[(i+1)%styles.length]+colors[(i+1)%colors.length]);
                label(pic, adapt_label(names[i+1], 0.5, DIM.x/2 - 0.5, E), (DIM.x/2+1.4,-0.5-i/8), align=E);
            }
        }
    } else {
        picture pp;
        unitsize(pp, 1cm);
        int k=0;
        real step = DIM.x/2+0.5;
        for (int i=0; i<names.length; ++i) {
            if (names[i] == "") {
                ++k;
                continue;
            }
            draw(pp, (step*(i-k),-0.5) -- (step*(i-k)+0.8,-0.5), styles[i%styles.length]+colors[i%colors.length]);
            label(pp, adapt_label(names[i], 0.5, DIM.x/2 - 0.5, E), (step*(i-k)+0.9,-0.5), align=E);
        }
        shipout(ppath+"-legenda", pp);
    }
    if (SUBPLOT && length(ppath) > 0) shipout(ppath, pic);
    return pic;
}

int x = 0;
int y = 0;
bool done = false;

// puts a plot in the next available grid position
void put(picture pic) {
    if (done) {
        newpage();
        done = false;
    }
    add(shift((x*(DIM.x+OFFS.x), -(DIM.y+OFFS.y)*y)) * pic);
    x += 1;
    if (x == COLS) {
        x = 0;
        y += 1;
    }
    if (y == ROWS) {
        y = 0;
        done = true;
    }
}
