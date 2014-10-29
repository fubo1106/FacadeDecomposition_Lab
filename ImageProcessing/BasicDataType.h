#pragma once 

#include <cv.h>
using namespace cv;
struct Segment{
	Point startp;
	Point endp;
	Segment(Point start,Point end){
		startp = start;
		endp = end;
	};
	Segment(){};
};

bool operator == ( Segment& A , Segment& B ){
	return 	(A.startp == B.startp && A.endp == B.endp);
}

struct Rec{
	Segment s1,s2,s3,s4;
	Point center;
};

struct Line{
	Line* father;
	Vec4i point;
	int h;
};