#pragma once

#include "stdafx.h"
#include "core.h"
#include "RectangleTools.h"
using namespace std;

void mergeSeg(Segment& src1,Segment src2,Segment& dst){
	if(src1.startp.x == src1.endp.x){//vertical
		dst.startp = Point( (src1.startp.x+src2.startp.x)/2, _min(src1.startp.y, src1.endp.y, src2.startp.y, src2.endp.y) );
		dst.endp = Point( (src1.startp.x+src2.startp.x)/2, _max(src1.startp.y, src1.endp.y, src2.startp.y, src2.endp.y) );
	}
	if(src1.startp.y == src1.startp.y){//horizontal
		dst.startp = Point( (src1.startp.y+src2.startp.y)/2, _min(src1.startp.x, src1.endp.x, src2.startp.x, src2.endp.x) );
		dst.endp = Point( (src1.startp.y+src2.startp.y)/2, _max(src1.startp.x, src1.endp.x, src2.startp.x, src2.endp.x) );
	}
	return;
}

void removeNonVertHorizonSegments(cv::vector<Segment>& lines, cv::vector<Segment>& reduced){
	/*reduce the line segments generate by Hough transform, conserve the horizontal and vertical
	reduced:storage  the vertical and horizontal segments*/
	double thresh = 0;
	Segment standardSeg;
	for(int i=0;i<lines.size();i++){
		if(abs(lines[i].startp.x - lines[i].endp.x)<= thresh)//vertical
		{
			standardSeg = Segment( Point( (lines[i].startp.x + lines[i].endp.x)/2,lines[i].startp.y),
								   Point((lines[i].startp.x + lines[i].endp.x)/2,lines[i].endp.y));
			reduced.push_back(standardSeg);
		}
			
		if(abs(lines[i].startp.y - lines[i].endp.y)<=thresh)//horizontal
		{
			standardSeg = Segment( Point(lines[i].startp.x,(lines[i].startp.y + lines[i].endp.y)/2),
								   Point(lines[i].endp.x,(lines[i].startp.y + lines[i].endp.y)/2));
			reduced.push_back(standardSeg);
		}
			
	}
	return;
}

void removeOverlappingSegments(vector<Segment>& inputs,vector<Segment>& reduced){
	/*reduce non-overlapping segments from vertical and horizontal line sets*/
	vector<Segment> h,v;
	vector<Segment> reduced_h,reduced_v;
	vector<int> remove_h,remove_v;

	double thresh = 20;

	for(int i=0;i<inputs.size();i++){
		if(abs(inputs[i].startp.x == inputs[i].endp.x) && abs(inputs[i].startp.y != inputs[i].endp.y))//vertical
			v.push_back(inputs[i]);
		if(abs(inputs[i].startp.y == inputs[i].endp.y) && abs(inputs[i].startp.x != inputs[i].endp.x))//horizontal
			h.push_back(inputs[i]);
	}

	for(int i=0;i<h.size()-1;i++)
		for(int j=i+1;j<h.size();j++){
			//distance<thresh && p1 p2 between p1'and p2'
			if( abs(h[i].startp.y-h[j].startp.y)<thresh && 
				(_min(h[i].startp.x,h[i].endp.x)<=_min(h[j].startp.x,h[j].endp.x))&&
				 _max(h[i].startp.x,h[i].endp.x)>=_max(h[j].startp.x,h[j].endp.x) ){
				if(notExist(j,remove_h))
					remove_h.push_back(j);
			}
			if( abs(h[i].startp.y-h[j].startp.y)<thresh && 
				(_min(h[j].startp.x,h[j].endp.x)<_min(h[i].startp.x,h[i].endp.x))&&
				 _max(h[j].startp.x,h[j].endp.x)>_max(h[i].startp.x,h[i].endp.x) ){
				if(notExist(i,remove_h))
					remove_h.push_back(i);
			}

	}

	for(int i=0;i<v.size()-1;i++)
		for(int j=i+1;j<v.size();j++){
			if( (abs(v[i].startp.x-v[j].startp.x)<thresh) && 
				(_min(v[i].startp.y,v[i].endp.y)<=_min(v[j].startp.y,v[j].endp.y) ) && 
				(_max(v[i].startp.y,v[i].endp.y)>=_max(v[j].startp.y,v[j].endp.y) )){
					/*cout<<_min(v[i][1],v[i][3])<<endl<<_min(v[j][1],v[j][3])<<endl
					<<_max(v[i][1],v[i][3])<<endl<<_max(v[j][1],v[j][3])<<endl;*/
				if(notExist(j,remove_v))
					remove_v.push_back(j);
			}
					
			if( (abs(v[i].startp.x-v[j].startp.x)<thresh) && 
				(_min(v[j].startp.y,v[j].endp.y)<_min(v[i].startp.y,v[i].endp.y) ) && 
				(_max(v[j].startp.y,v[j].endp.y)>_max(v[i].startp.y,v[i].endp.y) ) ){
				if(notExist(i,remove_v))
					remove_v.push_back(i);
			}
	}

		for(int i=0;i<h.size();i++){
			if(notExist(i,remove_h))
				reduced_h.push_back(h[i]);
		}
		for(int i=0;i<v.size();i++){
			if(notExist(i,remove_v))
				reduced_v.push_back(v[i]);
		}

		reduced.insert(reduced.end(),reduced_h.begin(),reduced_h.end());   
		reduced.insert(reduced.end(),reduced_v.begin(),reduced_v.end());   
		//reduced = v;
		return;
}

void symmetricCompletion(Mat& src, vector<Segment>& edges, vector<Rec>& rects){
	cv::Mat temp1;
	cv::Mat temp2;  // Your template image 
	cv::Mat result; // Result correlation will be placed here
	vector<Vec4i>h,v;
	vector<Segment> hseg,vseg;
	vector<Segment> merged_h,merged_v;
	double similarityThresh = 0.6;
	vector<Segment> symmetryed,symmetryed_h,symmetryed_v;
	for(int i=0;i<edges.size();i++){
		if(abs(edges[i].startp.x == edges[i].endp.x) && abs(edges[i].startp.y != edges[i].endp.y))//vertical
			vseg.push_back(edges[i]);
		if(abs(edges[i].startp.y == edges[i].endp.y) && abs(edges[i].startp.x != edges[i].endp.x))//horizontal
			hseg.push_back(edges[i]); 
	}

	//Vec4i2Segments(h,hseg);
	//Vec4i2Segments(v,vseg);
	/*Segment temp;
	for(int i=0;i<hseg.size()-1;)
		for(int j=i+1;j<hseg.size();j++){
			if(distSegment2Segment(hseg[i],hseg[j])[0] < 10){
				mergeSeg(hseg[i],hseg[j],temp);
				merged_h.push_back(temp);
			}
			else{
				merged_h.push_back(hseg[i]);
				i++;
				continue;
			}
		}

	for(int i=0;i<vseg.size()-1;)
		for(int j=i+1;j<vseg.size();j++){
			if(distSegment2Segment(vseg[i],vseg[j])[0] < 10){
				mergeSeg(vseg[i],vseg[j],temp);
				merged_v.push_back(temp);
			}
			else{
				merged_v.push_back(vseg[i]);
				i++;
				continue;
			}
		}*/

	/*for(int i=0;i<hseg.size()-1;i++)
		for(int j=i+1;j<hseg.size();j++)
			for(int k=0;k<vseg.size()-1;k++)
				for(int l=k+1;l<vseg.size();l++){
					Rec rec = findRectangle(hseg[i],hseg[j],vseg[k],vseg[l]);
					if(rec.center != Point(0,0)){
						rects.push_back(rec);
					}
				}*/
	
	vector<SegPair> hpairs,vpairs,pairs;
	vector<Segment> nopairs_h,nopairs_v,nopairs;
	SegPair pair;
	int symmetry_axis = src.cols/2;
	double thresh_symmetry = 10;
	//symmetryed_h = hseg; symmetryed_v = vseg;

	for(int i=0;i<hseg.size();){
		Segment sym_i = findSymmetrySegment(hseg[i],hseg,symmetry_axis,thresh_symmetry);
		if(sym_i.startp.x==0 && sym_i.startp.y==0 && sym_i.endp.x==0 && sym_i.endp.y==0){
			//symmetryed_h.push_back(calcSymmetrySegment(hseg[i],symmetry_axis));
			nopairs_h.push_back(hseg[i]);
			delete_SegVector(hseg,hseg[i]);
		}
		else{
			/*delete_SegVector(symmetryed_h,sym_i);
			delete_SegVector(symmetryed_h,hseg[i]);
			symmetryed_h.push_back(clc_ORset_SymmetrySegment(hseg[i],sym_i,symmetry_axis));
			if(sym_i != hseg[i]){
				symmetryed_h.push_back(clc_ORset_SymmetrySegment(sym_i,hseg[i],symmetry_axis));*/
			if(sym_i == hseg[i]){
				nopairs_h.push_back(clc_ORset_SymmetrySegment(sym_i,hseg[i],symmetry_axis));
				delete_SegVector(hseg,hseg[i]);
			}
			else{
				pair.s1 = sym_i;
				pair.s2 = hseg[i];
				delete_SegVector(hseg,sym_i);
				delete_SegVector(hseg,hseg[i]);
				hpairs.push_back(pair);
			}

		}
	}
	//}

	for(int i=0;i<vseg.size();){
		Segment sym_i = findSymmetrySegment(vseg[i],vseg,symmetry_axis,thresh_symmetry);
		if(sym_i.startp.x==0 && sym_i.startp.y==0 && sym_i.endp.x==0 && sym_i.endp.y==0){
			//symmetryed_v.push_back(calcSymmetrySegment(vseg[i],symmetry_axis));
			nopairs_v.push_back(vseg[i]);
			delete_SegVector(vseg,vseg[i]);
		}
		else{
			/*delete_SegVector(symmetryed_v,sym_i);
			delete_SegVector(symmetryed_v,vseg[i]);
			symmetryed_v.push_back(clc_ORset_SymmetrySegment(vseg[i],sym_i,symmetry_axis));
			if(sym_i != vseg[i]){
				symmetryed_v.push_back(clc_ORset_SymmetrySegment(sym_i,vseg[i],symmetry_axis));*/
			if(sym_i == vseg[i]){
				nopairs_v.push_back(clc_ORset_SymmetrySegment(sym_i,vseg[i],symmetry_axis));
				delete_SegVector(vseg,vseg[i]);
			}
			else{
				pair.s1 = sym_i;
				pair.s2 = vseg[i];
				delete_SegVector(vseg,sym_i);
				delete_SegVector(vseg,vseg[i]);
				vpairs.push_back(pair);
			}
			
		}
				
	}
	//}

	pairs.insert(pairs.end(),hpairs.begin(),hpairs.end());
	pairs.insert(pairs.end(),vpairs.begin(),vpairs.end());

	nopairs.insert(nopairs.end(),nopairs_h.begin(),nopairs_h.end());
	nopairs.insert(nopairs.end(),nopairs_v.begin(),nopairs_v.end());

	for(int i = 0;i<pairs.size();i++){
		Segment s1 = clc_ORset_SymmetrySegment(pairs[i].s1,pairs[i].s2,symmetry_axis);
		Segment s2 = clc_ORset_SymmetrySegment(pairs[i].s2,pairs[i].s1,symmetry_axis);
		symmetryed.push_back(s1);
		symmetryed.push_back(s2);
	}
	for(int i = 0;i<nopairs.size();i++){
		symmetryed.push_back(nopairs[i]);
	}

	//edges = symmetryed;
	/*merged_h.insert(merged_h.end(),merged_v.begin(),merged_v.end()); 
	edges = merged_h;*/

	vector<Segment> l_seg_h, r_seg_h,l_seg_v, r_seg_v;
	
	/*symmetryed.insert(symmetryed.end(),symmetryed_h.begin(),symmetryed_h.end());
	symmetryed.insert(symmetryed.end(),symmetryed_v.begin(),symmetryed_v.end());*/
	edges = symmetryed;

	/*classifySegsToLR(symmetryed_h, symmetry_axis, l_seg_h, r_seg_h);
	classifySegsToLR(symmetryed_v, symmetry_axis, l_seg_v, r_seg_v);*/

	classifySegsToHV(symmetryed, symmetryed_h,symmetryed_v);

	rects = findRectsFromHVSegs(symmetryed_h,symmetryed_v);
	
	//determine whether the nopairs segment should be added
	vector<Rec> potentialRecs = findPotentialRectsFromNoPairSegs(symmetry_axis,nopairs_h,nopairs_v,symmetryed_h,symmetryed_v);
 	rects.insert(rects.end(),potentialRecs.begin(),potentialRecs.end());
	
	vector<Rec> tempRec = rects;
	for(int i=0;i<tempRec.size();i++){
		Rec rec = findSymmetryRecFromRects(tempRec[i],tempRec,symmetry_axis,10);
		if(rec.center == Point(0,0))
			rects.push_back(calcSymmetryRectangle(tempRec[i],symmetry_axis));
	}


	//find the symmtric axis: originally set it as the middle of image
	temp1 = src(Range(0,src.rows),Range(0,src.cols/2));
	temp2 = src(Range(0,src.rows),Range(src.cols/2,src.cols));
	
	/*cv::namedWindow("temp1",0);
	cv::imshow("temp1",temp1);
	cv::waitKey(0);

	cv::namedWindow("temp2",0);
	cv::imshow("temp2",temp2);
	cv::waitKey(0);

	cv::namedWindow("src",0);
	cv::imshow("src",src);
	cv::waitKey(0);*/

	// Do template matching across whole image
	cv::matchTemplate(temp1, temp2, result, CV_TM_CCORR_NORMED);

	// Find a best match:
	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);

	if(maxVal >= similarityThresh){//center is the symmetric axis
		

	}

	// Move center of detected screw to the correct position:  
	//cv::Point screwCenter = maxLoc + cv::Point(temp1.cols/2, temp1.rows/2);
}

