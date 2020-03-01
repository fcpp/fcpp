include math;
settings.tex = "pdflatex";

pair DIM  = (5,3);   // size of a plot
pair OFFS = (2,2.5); // spacing between plots
int  ROWS = 2;       // rows of plots per page
int  COLS = 3;       // columns of plots per page
real MAX_CROP = 1.1; // maximum cropping allowed (usually 1.3)
real LOG_LIN = 2;    // factor of comparison between linear and logarithmic plots

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

// convert point coordinates into screen coordinates
real fit(real x, real a = 0, real b = 1, real l = 1, bool logmode = false) {
    return l*((logmode ? log10(max(x,realMin)) : x)-a)/(b-a);
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
        for (int j=0; j<values[i].length; ++j) {
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

    // scan y values for best area covered using linear plot
    if (valy.length > 0) {
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
                if (valy[i] != nan && area(lminy, lmaxy) < area(i, i+l)) {
                    lminy = i;
                    lmaxy = i+l;
                }
        for (int l=valy.length-1; MAX_CROP*l >= valy.length; --l)
            for (int i=0; i+l < valy.length; ++i)
                if (valy[i] != nan && 0.9 * area(lminy, lmaxy) < area(i, i+l)) {
                    lminy = i;
                    lmaxy = i+l;
                    i = valy.length;
                    l = 0;
                }
    // compare and choose the final plot window
        logmode = area(lminy, lmaxy) > besta * LOG_LIN;
        if (logmode) {
            bminy = valy[lminy]*0.999;
            bmaxy = valy[lmaxy]*1.001;
        } else {
            bminy = lvaly[iminy]*0.999;
            bmaxy = lvaly[imaxy]*1.001;
        }
        write(title + " linear: " + string(round(10000*besta)/100) + "% log: " + string(round(10000*area(lminy, lmaxy))/100) + "% (I choose " + (logmode ? "log)" : "lin)"));
    }
    if (bminy == bmaxy) {
        bminy = bminy - 0.5;
        bmaxy = bmaxy + 0.5;
    }
    real xscale = approx(bminx, bmaxx, DIM.x);
    real yscale = approx(bminy, bmaxy, DIM.y);

    for (int l=0; l<values.length; ++l) {
        if (values[l].length == 0) continue;
        path p;
        bool drawing = false;
        pair lp = (0,0);
        for (int i=0; i<values[l].length; ++i) {
            real x = fit(values[l][i].x, bminx, bmaxx, DIM.x);
            real y = fit(values[l][i].y, bminy, bmaxy, DIM.y, logmode);
            if (y != lp.y) {
                if (y < 0     || lp.y < 0    ) lp = (lp.x + (x-lp.x)*(0    -lp.y)/(y-lp.y), 0);
                if (y > DIM.y || lp.y > DIM.y) lp = (lp.x + (x-lp.x)*(DIM.y-lp.y)/(y-lp.y), DIM.y);
            }
            if (0 <= y && y <= DIM.y) {
                if (!drawing && i>0) { p = lp; if (lp.y < 0) write(string(x)+";"+string(y)+"|"+string(lp.x)+";"+string(lp.y)); }
                p = p -- (x,y);
                drawing = true;
            } else {
                if (drawing) draw(pic, p -- lp, styles[l%styles.length]+colors[l%colors.length]);
                drawing = false;
            };
            lp = (x,y);
        }
        if (drawing) draw(pic, p, styles[l%styles.length]+colors[l%colors.length]);
    }
    draw(pic, (0,0) -- (0,DIM.y+0.8), black, EndArrow(4));
    draw(pic, (0,0) -- (DIM.x+1.0,0), black, EndArrow(4));
    if (endx != 0) {
        endx = fit(endx, bminx, bmaxx, DIM.x);
        draw(pic, (endx,0) -- (endx,DIM.y), black+0.5);
    }
    if (length(xlabel) > 0)
        label(pic, scale(0.5)*("\textit{"+xlabel+"}"), (DIM.x+0.6,-0.15));
    if (length(ylabel) > 0)
        label(pic, rotate(90)*scale(0.5)*("\textit{"+ylabel+"}"), (-0.15,DIM.y+0.4));
    if (length(title) > 0)
        label(pic, scale(0.6)*title, (DIM.x/2+0.55,DIM.y+0.7));

    int xta = ceil( bminx/xscale);
    int xtb = floor(bmaxx/xscale);
    for (int x=xta; x <= xtb; ++x) {
        real rxs = x*xscale;
        real rx = DIM.x*(rxs-bminx)/(bmaxx-bminx);
        draw(pic, (rx,0) -- (rx,-.1));
        label(pic, scale(0.5)*string(rxs), (rx,-.2));
    }
    int yta = ceil( bminy/yscale);
    int ytb = floor(bmaxy/yscale);
    for (int y=yta; y <= ytb; ++y) {
        real rys = y*yscale;
        real ry = fit(rys, bminy, bmaxy, DIM.y);
        draw(pic, (0,ry) -- (-.1,ry));
        string s = string(rys);
        label(pic, scale(0.5)*(logmode ? "$10^{"+s+"}$" : s), (0,ry), align=W);
    }
    for (int i=0; i<names.length; i+=2) {
        draw(pic, (0,-0.5-i/8) -- (0.8,-0.5-i/8), styles[i%styles.length]+colors[i%colors.length]);
        label(pic, scale(0.5)*(names[i]+"\phantom{pd}"), (0.9,-0.5-i/8), align=E);
        if (i+1 < names.length) {
            draw(pic, (DIM.x/2+0.5,-0.5-i/8) -- (DIM.x/2+1.3,-0.5-i/8), styles[(i+1)%styles.length]+colors[(i+1)%colors.length]);
            label(pic, scale(0.5)*(names[i+1]+"\phantom{pd}"), (DIM.x/2+1.4,-0.5-i/8), align=E);
        }
    }
    if (length(ppath) > 0) shipout(ppath, pic);
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
