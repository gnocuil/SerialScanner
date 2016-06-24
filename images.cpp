#include <string>
#include <iostream>
#include <CImg.h>
#include <queue>
#include <vector>
#include "images.h"
#include "ocr.h"
#include "timer.h"

#define SHOW_ROTATE 1

using namespace std;

const int MXD = 3;

const int BLACK = 0;
const int WHITE = 1;

static void setColor(CImg<unsigned char>& img, int w, int h, int color)
{
    for (int i = 0; i < img.spectrum(); ++i)
        img(w,h,i) = color;
}

Image::Image(string filename)
{
    debug = 0;
    threshold = 140;//TODO: default threshold
    printf("Image file %s\n", filename.c_str());
    image = CImg<unsigned char>(filename.c_str());
    //printf("ok width=%d height=%d depth=%d spectrum=%d\n", image.width(), image.height(), image.depth(), image.spectrum());
    //if (image.width()>image.height())
    //    image.rotate(90);
    
    //image.resize_halfXY();
    
    int w=image.width();
    int h=image.height();
    
    //s_h = h;
    //s_w = w;
    //s = new int*[h];

    for (int i = 0; i < h; ++i) {
        //s[i] = new int[w];
        for (int j = 0; j < w; ++j) {
            int gray;
            if (image.spectrum() == 3) {
                int r = image(j,i,0,0);
                int g = image(j,i,0,1);
                int b = image(j,i,2);
                gray = (r*299 + g*587 + b*114 + 500) / 1000;
            } else {
                gray = image(j,i,0,0);
            }
            
            //const int D = 100;
            //if (r>g+D or r>b+D or g>r+D or g>b+D or b>r+D or b>g+D)
            //    gray = 255;
            
            //s[i][j]=(int)gray;
            setColor(image,j,i,gray);
        }
    }

    /*
    for (int i = 0; i < h; ++i) {
        //image(0,i,0) = 0;
        //image(0,i,1) = 0;
        //image(0,i,2) = 0;
        image(image.width()-1,i,0) = 0;
        image(image.width()-1,i,1) = 0;
        image(image.width()-1,i,2) = 0;
    }
    */
    prefix = "debug/1_";
    
    //image.display("test1");
    //if (debug) image.save((prefix+"_gray.jpg").c_str());
}

Image::Image(const CImg<unsigned char>& img_, Image* parent, string prefix_)
{
    //s=NULL;
    image.assign(img_);
    
    prefix = parent->prefix + prefix_;
    threshold = parent->threshold;
    debug = parent->debug;
}

Image Image::subImage(int h1, int h2, int id)
{
    char ids[100] = {0};
    sprintf(ids, "sub%d_", id);
    Image img(image, this, ids);
    int w = image.width();
    img.image.crop(0,h1,0,0,w-1,h2,0,img.image.spectrum()-1);
    return img;
}

Image::~Image()
{
    /*
    printf("free image\n");
    if (s) {
        for (int i = 0; i < s_h; ++i)
            delete[] s[i];
        delete[] s;
        s = NULL;
    }
    printf("free image #2\n");
    */
}


vector<string> Image::search(int threshold_)
{
    vector<string> serials;
    threshold = threshold_;
    int w=image.width();
    int h=image.height();
    printf("search t=%d w=%d h=%d\n", threshold, w, h);
    int **bi = new int*[h];
    for (int i = 0; i < h; ++i) {
        bi[i] = new int[w];
        for (int j = 0; j < w; ++j) {
            if (image(j,i,0) < threshold)
                bi[i][j] = BLACK;
            else
                bi[i][j] = WHITE;
        }
    }


    //TODO: replace all existing rects
    CImg<unsigned char> bimg(image);
    for (int i = 0; i < h; ++i) {
        for (int j = 0; j < w; ++j) {
            int v = 0;
            if (bi[i][j]==WHITE) v=255;
            setColor(bimg, j, i, v);
        }
    }
    if (debug) image.save("debug/_gray.jpg");
    if (debug) bimg.save("debug/_binary.jpg");
    
    Timer tbfs;
    int tot = 0;
    double best = 0;
    int bw1=-1, bw2, bh1, bh2;
    for (int i = 0; i < h; ++i)
        for (int j = 0; j < w; ++j) if (bi[i][j]==BLACK) {
            //printf("try %d %d\n",i,j);
            queue<Point> q;
            q.push(Point(j,i));
            bi[i][j]=WHITE;
            int cnt=1;
            int w1=j,w2=j,h1=i,h2=i;
            while (!q.empty()) {
                Point cur = q.front();
                q.pop();
                //printf("cur %d %d\n",cur.w,cur.h);
                if (cur.w<w1) w1=cur.w;
                if (cur.w>w2) w2=cur.w;
                if (cur.h<h1) h1=cur.h;
                if (cur.h>h2) h2=cur.h;

                for (int dx = -MXD; dx <= MXD; ++dx)
                    for (int dy = -MXD; dy <= MXD; ++dy) {
                        if (dx==0 && dy==0) continue;
                        int newh = cur.h + dx;
                        int neww = cur.w + dy;
                        if (neww>=w || neww<0) continue;
                        if (newh>=h || newh<0) continue;
                        if (bi[newh][neww]!=BLACK) continue;
                        bi[newh][neww]=WHITE;
                        q.push(Point(neww,newh));
                        ++cnt;
                        //printf("push %d %d\n",neww,newh);
                    }
            }
            //printf("test: cnt=%d\n",cnt);
            int area=(w2-w1+1)*(h2-h1+1);
            if (area < 1000) continue;

            double ratio = (double)(w2-w1+1)/(h2-h1+1);
            double per = (double)area / (w*h);
            double sparse = (double)area/cnt;
            
            if (sparse < 6) continue;//should be sparse
            //if (per < 0.04) continue;//compare with WHOLE picture witout cropping
            if (per > 0.5) continue;//compare with WHOLE picture witout cropping
            if (ratio < 2) continue;//should be a LONG area
            //printf("        %d: cnt=%d area=%d ratio=%.2lf per=%.5lf sparse=%.2lf\n",tot,cnt,area,ratio,per,sparse);
            printf("found area! tot=%d area=%d cnt=%d sparse=%.2lf per=%.2lf ratio=%.2lf\n",tot,area,cnt,sparse,per,ratio);  
            //tbfs.current("BFS");
            char prefix[100] = {0};
            sprintf(prefix, "part%d_", tot++);
            
            CImg<unsigned char> img2(image);
            
            //img2.save((this->prefix+prefix+"img2-1.jpg").c_str());
            bool clearup = false;
            bool cleardown = false;
            if (h1>0) {
                clearup = true;
                h1--;
            }
            if (h2+1<h) {
                cleardown = true;
                h2++;
            }
            img2.crop(w1,h1,0,0,w2,h2,0,img2.spectrum()-1);

            
            if (clearup) {
                for (int j = 0; j < img2.width(); ++j) {
                    setColor(img2, j, 0, 255);
                }
            }
            
            if (cleardown) {
                for (int j = 0; j < img2.width(); ++j) {
                    setColor(img2, j, img2.height()-1, 255);
                }
            }
            
            Image subimg(img2, this, prefix);
            //puts("## before recognize");
            string ret = subimg.recognize();
            //puts("## after recognize");
            //cout << "over w,h="<<w<<","<<h<<" : ["<<ret<<"]"<<endl;
            if (ret.size() > 0) {
                serials.push_back(ret);
                for (int i2 = h1; i2 <= h2; ++i2)
                    for (int j2 = w1; j2 <= w2; ++j2) {
                        bi[i2][j2]=WHITE;
                        setColor(image, j2, i2, 255);
                    }
            }
            tbfs.reset();
            
            //sprintf(tt,"debug/3_part_%d.jpg", tot++);
            //img2.save(tt);
            //img2.display(tt);
        }
    //printf("finished loop\n");

        
    for (int i = 0; i < h; ++i) {
        delete[] bi[i];
    }
    delete[] bi;
    //printf("return...\n");
    return serials;
}

int Image::countLine(const CImg<unsigned char>& img, int line)
{
    int i = line;
    int cnt = 0;
    for (int j = 0; j < img.width(); ++j) {
        int gray;
        if (img.spectrum() == 3) {
            int r = img(j,i,0,0);
            int g = img(j,i,0,1);
            int b = img(j,i,0,2);
            gray = (r*299 + g*587 + b*114 + 500) / 1000;
        } else
            gray = img(j,i,0,0);
        if (gray < threshold)//TODO: threshold?
            cnt++;
    }
    return cnt;
}

int Image::count(const CImg<unsigned char>& img)
{
    //FILE *fout=fopen((prefix+".txt").c_str(),"w");
    int max = 0;
    for (int i = 0; i < img.height(); ++i) {
        int cnt = countLine(img, i);
        if (cnt>max) max=cnt;
        //fprintf(fout, "%d: %d / %d (%.4lf)\n", i, cnt, image.width(), cnt*1.0/image.width());
    }
    
    //fclose(fout);
    return max;
}

void Image::rotate()
{/*
    const double RANGE = 5;
    double step = 0.2;
    
    int mx = count(image);
    double best = 0;
    CImg<unsigned char> img_m = image.get_rotate(-step, 1, 1);
    int minus = count(img_m);
    CImg<unsigned char> img_p = image.get_rotate(step, 1, 1);
    int plus = count(img_p);
    double start = -RANGE;
    double end = RANGE;
    bool mi = false;
    if (minus > plus) {
        mi = true;
        if (minus > mx) {
            mx = minus;
            best = -step;
        }
    } else {
        if (plus > mx) {
            mx = plus;
            best = step;
        }
    }
    int decrease = 0;
    for (double d = step*2; d <= RANGE; d+=step) {
        double cur = d;
        if (mi) cur=-d;
        CImg<unsigned char> img2 = image.get_rotate((float)cur, 1, 1);
        int cnt = count(img2);
        if (cnt>mx) {
            mx = cnt;
            best = d;
        }
        if (cnt < mx) {
            ++decrease;
            if (decrease > 3)
                break;
        }
    }
    if (fabs(best)>1e-5) {
        image.rotate((float)best, 1, 1);
    }*/
    
    const double RANGE = 3;
    double step = 0.2;
    
    CImg<unsigned char> smallimg;
    
    if (image.width() > 1000)
        smallimg = image.get_resize_halfXY();
    else
        smallimg = image;
    
    while (smallimg.width() > 500)
        smallimg.resize_halfXY();
    
    //cout << "SMALLIMG size="<<smallimg.width()<<"x"<<smallimg.height()<<"  ("<<image.width()<<"x"<<image.height()<<")"<<endl;
    
    int mx = 0;
    double best = 0;
    for (double d = -RANGE; d <= RANGE; d+=step) {
        double cur = d;
        CImg<unsigned char> img2 = smallimg.get_rotate((float)cur, 1, 1);
        int cnt = count(img2);
        if (cnt>mx) {
            mx = cnt;
            best = d;
        }
        //printf("ROT: d=%.2lf  cnt=%d\n", d, cnt);
    }
    if (fabs(best)>1e-5) {
        image.rotate((float)best, 1, 1);
    }
}

vector<Image> Image::cut()
{
    vector<Image> ret;
    //FILE *fout=fopen((prefix+"2_.txt").c_str(),"w");
    
    //image.crop(0,h/6,0,0,w-1-w/12,h-1-h/7,0,image.spectrum()-1);
    int last = 0;
    int id = 0;
    for (int i = 0; i < image.height(); ++i) {
        int cnt = countLine(image, i);
        //fprintf(fout, "%d: %d / %d (%.4lf)\n", i, cnt, image.width(), cnt*1.0/image.width());
        if (cnt*2>image.width()) {
            if (i - last + 1 > 10) {//count
                //printf("found last=%d cur=%d id=%d\n", last, i, id);
                ret.push_back(subImage(last, i, id));
                id++;
            }
            last = i;
        }
    }
    //fclose(fout);
    if (image.height() - 1 - last + 1 > 10) {
        ret.push_back(subImage(last, image.height() - 1, id));
        id++;
    }
    return ret;
}

string Image::recognize()
{
    //int w=image.width();
    //int h=image.height();
    cout<<"    recognize: "<<(prefix+".jpg")<<endl;
    if (SHOW_ROTATE or debug) save("orig");
    //countLine();
    
    //TODO: rotate
    
    //image.rotate(-3, 1, 1);
    //cout<<"before rotate: "<<(prefix+".jpg")<<endl;
    Timer t1;
    rotate();
    ///t1.current("Rotate image");
    //w=image.width();
    //h=image.height();
    if (SHOW_ROTATE or debug) save("rotate");
    //cout<<"after rotate: "<<(prefix+".jpg")<<endl;
    
    //TODO: cut
    Timer t2;
    vector<Image> vi = cut();
    //t2.current("Cut image");
    printf("    cut images into: %d parts\n", (int)vi.size());
    for (int i = 0; i < vi.size(); ++i) {
        //string filename=vi[i].prefix+"T1.jpg";
        //cout<<"FN: "<<filename<<endl;
        //vi[i].image.save(filename.c_str());
        Timer t3;
        string cur = vi[i].recognize_2();
        //t3.current("Recognize_2");
        if (cur.size()>0) return cur;
    }
    
    //image.save((prefix+"_3_crop.jpg").c_str());
    return "";
}

static int getTopPixes(const CImg<unsigned char>& img)
{
    int sum = 0;
    for (int hh = 0; hh < img.height()/6; ++hh)
        for (int ww = 0; ww < img.width(); ++ww)
            if (img(ww,hh,0)<127) ++sum;
    return sum;
}

static int getDownPixes(const CImg<unsigned char>& img)
{
    int sum = 0;
    for (int hh = img.height()-img.height()/8; hh < img.height(); ++hh)
        for (int ww = 0; ww < img.width(); ++ww)
            if (img(ww,hh,0)<127) ++sum;
    return sum;
}

string Image::recognize_2()
{
    int w=image.width();
    int h=image.height();
    if (h*3>w) return "";
    //image.crop(0,h/6,0,0,w-1-w/12,h-1-h/7,0,image.spectrum()-1);
    image.crop(0,h/6,0,0,w-1,h-1-h/7,0,image.spectrum()-1);
    string filename=prefix+"#orig.jpg";
    //cout << "FN: "<<filename<<' '<<image.width()<<" "<<image.height()<<endl;
    //image.save(filename.c_str());
    
    //binary
    for (int i = 0; i < image.height(); ++i)
        for (int j = 0; j < image.width(); ++j) {
            int gray;
            if (image.spectrum()==3) {
                int r = image(j,i,0,0);
                int g = image(j,i,0,1);
                int b = image(j,i,0,2);
                gray = (r*299 + g*587 + b*114 + 500) / 1000;
            } else
                gray = image(j,i,0,0);
            int color = 255;
            if (gray < threshold)
                color = 0;
            setColor(image,j,i,color);
        }
    if (debug) save("#bina");
    
    //cut left-up
    
    //find 16 charas
    
    findChar();
    
    if (debug) save("#bina-crop");
    
    //ocr
    string ret = "";
    int cnt = 0;
    int leftw[20] = {-1};
    int rightw[20];
    for (int j = 0; j < image.width(); ++j) {
        if (cnt > NUM1+NUM2) break;
        bool black = false;
        for (int i = 0; i < image.height(); ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (black) {
            if (leftw[cnt] < 0) leftw[cnt] = j;
            rightw[cnt] = j;
        } else {
            if (leftw[cnt] >= 0) {
                cnt++;
                leftw[cnt] = -1;
            }
        }
    }
    if (leftw[cnt] >= 0) ++cnt;
    if (cnt != NUM1+NUM2) {
        printf("    number of char not match 8+8, skip OCR!\n");
        return "";
    }
    
    Timer tt;
    string line16 = ocr16(image);
    //tt.current("ocr16");
    if (line16.size() > 0)
        cout << "    OCR LINE : "<<line16<<endl;  
    else {
        cout << "    Failed OCR"<<line16<<endl;  
        return "";
    }
    string oldline = line16;
    //printf("# OCR debug #1, cnt=%d\n",cnt);
    int nxt = 0;
    for (int i = 0; i < cnt; ++i) {
        //printf("i=%d width=%d left=%d right=%d\n", i, image.width(), leftw[i], rightw[i]);
        CImg<unsigned char> img = image.get_crop(leftw[i],0,0,0,rightw[i],image.height()-1,0,image.spectrum()-1);
        if (i==8) ++nxt;
        if (line16[nxt] == 'u' and img.width()>2 and img.height()>2) {
            bool w = false;
            if (img(img.width()/2,img.height()/2,0)<127) w = true;
            if (img(img.width()/2-1,img.height()/2,0)<127) w = true;
            if (img(img.width()/2,img.height()/2+1,0)<127) w = true;
            if (img(img.width()/2-1,img.height()/2+1,0)<127) w = true;
            if (w)
                line16[nxt] = 'w';
        } else if (line16[nxt] == '5' or line16[nxt] == 's') {
            if (getTopPixes(img)==0)
                line16[nxt] = 's';
            else
                line16[nxt] = '5';
        } else if (line16[nxt] == 'z' or line16[nxt] == '2') {
            if (getTopPixes(img)==0)
                line16[nxt] = 'z';
            else
                line16[nxt] = '2';
        } else if (line16[nxt] == 'g' or line16[nxt] == '9') {
            if (getTopPixes(img)==0)
                line16[nxt] = 'g';
            else
                line16[nxt] = '9';
        } else if (line16[nxt] == 'i') {
            if (getDownPixes(img)>0)
                line16[nxt] = 'j';
        }
        ++nxt;
        if (debug) {
            char tt[100] = {0};
            sprintf(tt, "char%02d.jpg", i);
            img.save((prefix+tt).c_str());
        }
    }
    if (oldline != line16)
        cout << "      Result fix : "<<line16<<endl;  
   
        //printf("# OCR debug #2\n");
    //check
    
    return line16;
}

void Image::findChar()
{
    //puts("findchar!");
    int h = image.height();
    int w = image.width();
    int midh = h/2;
    int midw = w/2;
    
    int mx = 0;
    int i, j;
    
    int ups = 0;
    int left_cut = 0;
    for (int j = 0; j < w; ++j) {
        int up = 0;
        int down = 0;
        for (int i = 0; i < h; ++i) {
            if (image(j,i,0)<127) {
                if (i<h/4) ++up;
                else ++down;
            }
        }
        //cout << "    UP " << j << " " << up << " " << down << " " << ups << endl;
        if (up+down>0) {
            if (down == 0) {
                ++ups;
                left_cut=j+1;
                
            }
            if (down > 0) {
                if (ups > 8) {
                    if (debug) cout << "    LEFT_CUT: " << prefix << " " << left_cut << " "<< ups << endl;
                    image.crop(left_cut,0,0,0,w-1,h-1,0,image.spectrum()-1);
                    h = image.height();
                    w = image.width();
                    midh = h/2;
                    midw = w/2;
                    break;
                }
                ups = 0;
            }
        }
    }
    
    
    for (i = midh; i >= 0; --i) {
        int cnt = 0;
        for (j = 0; j < w; ++j)
            if (image(j,i,0)<127) ++cnt;
        if (mx > 0 && i < midh-1 && cnt*10<mx) break;
        if (cnt > mx) mx = cnt;
    }
    int top = i+1;
    //printf("top=%d\n", top);

    for (i = midh; i < h; ++i) {
        int cnt = 0;
        for (j = 0; j < w; ++j)
            if (image(j,i,0)<127) ++cnt;
        if (mx > 0 && i + midh+1 && cnt*10<mx) break;
        if (cnt > mx) mx = cnt;
    }
    int down = i-1;
    //printf("down=%d\n", down);
    
    int last = -1;
    int mx1 = 0, mx2 = 0, mx3 = 0;
    int len = 0, mxlen = 0;
    for (j = midw - w / 4; j < w; ++j) {
        bool black = false;
        for (i = top; i <= down; ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (black) {
            ++len;
            if (len>mxlen) mxlen=len;
            if (last >= 0) {
                int delta = j - last - 1;
                if (mx3 > 0 && delta>mxlen*3) break;
                if (delta > mx1) {
                    mx3 = mx2;
                    mx2 = mx1;
                    mx1 = delta;
                } else if (delta > mx2) {
                    mx3 = mx2;
                    mx2 = delta;
                } else if (delta > mx3) {
                    mx3 = delta;
                }
            }
            last = j;
        } else {
            len = 0;
        }
    }
    //printf("mx1=%d mx2=%d\n", mx1, mx2);
    
    last = midw + w / 4;
    for (j = last; j < w; ++j) {
        bool black = false;
        for (i = h/4; i <= down; ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        
        if (black) {
            last = j;
        } else if (j - last - 1 > mx2*2)
            break;
    }
    int right = j - 1;
    //printf("right=%d\n", right);
    
    last = midw - w / 4;
    for (j = last; j >= 0; --j) {
        bool black = false;
        for (i = h/4; i <= down; ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (black) {
            last = j;
        } else if (last - j - 1 > mx2*2)
            break;
    }
    int left = j + 1;
    //printf("left=%d\n", left);
    
    for (i = midh; i >= 0; --i) {
        bool black = false;
        for (j = left; j <= right; ++j)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (!black) break;
    }
    top = i + 1;
    
    for (i = midh; i < h; ++i) {
        bool black = false;
        for (j = left; j <= right; ++j)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (!black) break;
    }
    down = i - 1;
    
    while (left < w && left < right) {
        bool black = false;
        for (i = top; i <= down; ++i)
            if (image(left,i,0)<127) {
                black = true;
                break;
            }
        if (black) break;
        ++left;
    }
  
    while (right >= 0 && left < right) {
        bool black = false;
        for (i = top; i <= down; ++i)
            if (image(right,i,0)<127) {
                black = true;
                break;
            }
        if (black) break;
        --right;
    }
    //printf("l-t-r-d %d %d %d %d\n", left, top, right, down);
    image.crop(left,top,0,0,right,down,0,image.spectrum()-1);
}

void Image::save(string suffix)
{
    image.save((prefix+suffix+".jpg").c_str());
}

char Image::ocr(const CImg<unsigned char>& img)
{
    img.save(ocr1.path.c_str());
    string ret = ocr1.scan();
    if (debug) cout << "    OCR:{" << ret << "}" << endl;
    if (ret.size() >= 1) {
        if (!isalnum(ret[0])) return 0;
    }
    return ret[0];
}

string Image::ocr16(const CImg<unsigned char>& img)
{
    img.save(ocr1.path.c_str());
    string line = ocr1.scan16();
    while (line.size()>0 && isspace(line[line.size()-1]))
        line.resize(line.size()-1);
    if (debug) cout << "    OCR:{" << line << "}" << endl;
    string ret = "";
    for (int i = 0; i < line.size(); ++i) {
        if (isalnum(line[i])) {
            if (ret.size()==8) return "";
            ret += line[i];
        } else if (line[i] == ' ' && ret.size()==8) ret+=',';
        else if (!isspace(line[i])) return "";
    }
    if (ret.size() != NUM1+NUM2+1) return "";
    return ret;
}
