#include <string>
#include <iostream>
#include <CImg.h>
#include <queue>
#include <vector>
#include "images.h"
#include "ocr.h"

using namespace std;

const int MXD = 3;

const int BLACK = 0;
const int WHITE = 1;

Image::Image(string filename)
{
    threshold = 140;//TODO: default threshold
    printf("Image file %s\n", filename.c_str());
    image = CImg<unsigned char>(filename.c_str());
    printf("ok\n");
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
            
            int r = image(j,i,0,0);
            int g = image(j,i,0,1);
            int b = image(j,i,0,2);
            int gray = (r*299 + g*587 + b*114 + 500) / 1000;
            
            const int D = 100;
            //if (r>g+D or r>b+D or g>r+D or g>b+D or b>r+D or b>g+D)
            //    gray = 255;
            
            //s[i][j]=(int)gray;
            
            image(j,i,0)=gray;
            image(j,i,1)=gray;
            image(j,i,2)=gray;
            
        }
    }
    prefix = "debug/1_";
    
    //image.display("test1");
    image.save((prefix+"gray.jpg").c_str());
}

Image::Image(CImg<unsigned char> img_, Image* parent, string prefix_)
{
    //s=NULL;
    image.assign(img_);
    
    prefix = parent->prefix + prefix_;
    threshold = parent->threshold;
}

Image Image::subImage(int h1, int h2, int id)
{
    char ids[100] = {0};
    sprintf(ids, "sub%d_", id);
    Image img(image, this, ids);
    int w = image.width();
    img.image.crop(0,h1,0,0,w-1,h2,0,2);
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


string Image::search(int threshold_)
{
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
            bimg(j,i,0)=v;
            bimg(j,i,1)=v;
            bimg(j,i,2)=v;
        }
    }
    bimg.save("debug/2_binary.jpg");
    
    
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
            //if (ratio < 4) continue;//should be a LONG area
            //printf("        %d: cnt=%d area=%d ratio=%.2lf per=%.5lf sparse=%.2lf\n",tot,cnt,area,ratio,per,sparse);
            printf("found! tot=%d area=%d cnt=%d sparse=%.2lf per=%.2lf ratio=%.2lf\n",tot,area,cnt,sparse,per,ratio);  
            
            CImg<unsigned char> img2(image);
            img2.crop(w1,h1,0,0,w2,h2,0,2);
            char prefix[100] = {0};
            sprintf(prefix, "part%d_", tot++);
            Image subimg(img2, this, prefix);
            
            subimg.recognize();
            
            printf("over w=%d h=%d\n",w,h);
            
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
    return "";
}

int Image::countLine(const CImg<unsigned char>& img, int line)
{
    int i = line;
    int cnt = 0;
    for (int j = 0; j < img.width(); ++j) {
        int r = img(j,i,0,0);
        int g = img(j,i,0,1);
        int b = img(j,i,0,2);
        int gray = (r*299 + g*587 + b*114 + 500) / 1000;
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
{
    const double RANGE = 5;
    const double step = 0.1;
    int mx = 0;
    double best = 0;
    for (double d = -RANGE; d <= RANGE; d+=step) {
        CImg<unsigned char> img2 = image.get_rotate((float)d, 1, 1);
        int cur = count(img2);
        if (cur>mx) {
            mx = cur;
            best = d;
        }
    }
    if (fabs(best)>1e-5) {
        image.rotate((float)best, 1, 1);
    }
}

vector<Image> Image::cut()
{
    vector<Image> ret;
    FILE *fout=fopen((prefix+"2_.txt").c_str(),"w");
    
    //image.crop(0,h/6,0,0,w-1-w/12,h-1-h/7,0,2);
    int last = 0;
    int id = 0;
    for (int i = 0; i < image.height(); ++i) {
        int cnt = countLine(image, i);
        fprintf(fout, "%d: %d / %d (%.4lf)\n", i, cnt, image.width(), cnt*1.0/image.width());
        if (cnt*2>image.width()) {
            if (i - last + 1 > 10) {//count
                printf("found last=%d cur=%d id=%d\n", last, i, id);
                ret.push_back(subImage(last, i, id));
                id++;
            }
            last = i;
        }
    }
    fclose(fout);
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
    cout<<"recognize: "<<(prefix+".jpg")<<endl;
    image.save((prefix+"1_orig.jpg").c_str());
    //countLine();
    
    //TODO: rotate
    
    //image.rotate(-3, 1, 1);
    rotate();
    //w=image.width();
    //h=image.height();
    image.save((prefix+"2_rotate.jpg").c_str());
    
    //TODO: cut
    vector<Image> vi = cut();
    printf("cut images: %d\n", vi.size());
    for (int i = 0; i < vi.size(); ++i) {
        //string filename=vi[i].prefix+"T1.jpg";
        //cout<<"FN: "<<filename<<endl;
        //vi[i].image.save(filename.c_str());
        string cur = vi[i].recognize_2();
        if (cur.size()>0) return cur;
    }
    
    //image.save((prefix+"_3_crop.jpg").c_str());
    return "";
}

string Image::recognize_2()
{
    int w=image.width();
    int h=image.height();
    if (h*3>w) return "";
    image.crop(0,h/6,0,0,w-1-w/12,h-1-h/7,0,2);
    string filename=prefix+"#orig.jpg";
    cout << "FN: "<<filename<<' '<<image.width()<<" "<<image.height()<<endl;
    image.save(filename.c_str());
    
    //binary
    for (int i = 0; i < image.height(); ++i)
        for (int j = 0; j < image.width(); ++j) {
            int r = image(j,i,0,0);
            int g = image(j,i,0,1);
            int b = image(j,i,0,2);
            int gray = (r*299 + g*587 + b*114 + 500) / 1000;
            int color = 255;
            if (gray < threshold)
                color = 0;
            image(j,i,0) = color;
            image(j,i,1) = color;
            image(j,i,2) = color;
        }
    save("#bina");
    
    //cut left-up
    
    //find 16 charas
    
    findChar();
    
    save("#bina-crop");
    
    //ocr
    int left[20] = {-1};
    int right[20];
    int cnt = 0;
    for (int j = 0; j < image.width(); ++j) {
        bool black = false;
        for (int i = 0; i < image.height(); ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (black) {
            if (left[cnt] < 0) left[cnt] = j;
            right[cnt] = j;
        } else {
            if (left[cnt] >= 0) {
                cnt++;
                left[cnt] = -1;
            }
        }
    }
    if (left[cnt] >= 0) ++cnt;
    if (cnt != NUM1 + NUM2) {
        printf("number length not match! failed...\n");
        return "";
    }
    cout << "begin OCR! prefix="<<prefix<<endl;
    string ret = "";
    for (int i = 0; i < cnt; ++i) {
        CImg<unsigned char> img = image.get_crop(left[i],0,0,0,right[i],image.height()-1,0,2);
        char tt[100] = {0};
        sprintf(tt, "char%02d.jpg", i);
        img.save((prefix+tt).c_str());
        char ch = ocr(img);
        if (ch == 0) {
            printf("invalid char at pos %d, failed...\n", i);
            return "";
        }
        ret += ch;
        if (ret.size() == 8) ret+=',';
    }
    
    
    //check
    
    return ret;
}

void Image::findChar()
{
    puts("findchar!");
    int h = image.height();
    int w = image.width();
    int midh = h/2;
    int midw = w/2;
    
    int mx = 0;
    int i, j;
    
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
    int mx1 = 0, mx2 = 0;
    for (j = midw - w / 4; j < w; ++j) {
        bool black = false;
        for (i = top; i <= down; ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (black) {
            if (last >= 0) {
                int delta = j - last - 1;
                if (delta > mx1) {
                    mx2 = mx1;
                    mx1 = delta;
                } else if (delta > mx2) {
                    mx2 = delta;
                }
            }
            last = j;
        }
    }
    //printf("mx1=%d mx2=%d\n", mx1, mx2);
    
    last = midw + w / 4;
    for (j = last; j < w; ++j) {
        bool black = false;
        for (i = top; i <= down; ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (j - last - 1 > mx2*2) break;
        if (black) {
            last = j;
        }
    }
    int right = j - 1;
    //printf("right=%d\n", right);
    
    last = midw - w / 4;
    for (j = last; j >= 0; --j) {
        bool black = false;
        for (i = top; i <= down; ++i)
            if (image(j,i,0)<127) {
                black = true;
                break;
            }
        if (last - j - 1 > mx2*2) break;
        if (black) {
            last = j;
        }
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
    printf("l-t-r-d %d %d %d %d\n", left, top, right, down);
    image.crop(left,top,0,0,right,down,0,2);
}

void Image::save(string suffix)
{
    image.save((prefix+suffix+".jpg").c_str());
}

char Image::ocr(const CImg<unsigned char>& img)
{
    img.save(ocr1.path.c_str());
    string ret = ocr1.scan();
    cout << "OCR:{" << ret << "}" << endl;
    if (ret.size() >= 1) {
        if (!isalnum(ret[0])) return 0;
    }
    return ret[0];
}
