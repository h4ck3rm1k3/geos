/**********************************************************************
 * $Id$
 *
 * GEOS - Geometry Engine Open Source
 * http://geos.refractions.net
 *
 * Copyright (C) 2006 Refractions Research Inc.
 *
 * This is free software; you can redistribute and/or modify it under
 * the terms of the GNU Lesser General Licence as published
 * by the Free Software Foundation. 
 * See the COPYING file for more information.
 *
 **********************************************************************
 *
 * Last port: noding/snapround/SimpleSnapRounder.java rev. 1.2 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/noding.h>
#include <geos/nodingSnapround.h>

namespace geos {

/*private*/
void
SimpleSnapRounder::checkCorrectness(SegmentString::NonConstVect& inputSegmentStrings)
{
	auto_ptr<SegmentString::NonConstVect> resultSegStrings(
		SegmentString::getNodedSubstrings(inputSegmentStrings)
	);

	NodingValidator nv(*(resultSegStrings.get()));
	try {
		nv.checkValid();
	} catch (const std::exception &ex) {
		std::cerr<<ex.what();
	}
 
}

/*private*/
void
SimpleSnapRounder::computeSnaps(const SegmentString::NonConstVect& segStrings,
		vector<Coordinate>& snapPts)
{
	for (SegmentString::NonConstVect::const_iterator
			i=segStrings.begin(), iEnd=segStrings.end();
			i!=iEnd; ++i)
	{
		SegmentString* ss = *i;
		computeSnaps(ss, snapPts);
	}
}

/*private*/
void
SimpleSnapRounder::computeSnaps(SegmentString* ss, vector<Coordinate>& snapPts)
{
	for (vector<Coordinate>::iterator it=snapPts.begin(), itEnd=snapPts.end();
			it!=itEnd; ++it)
	{
		const Coordinate& snapPt = *it;
		HotPixel hotPixel(snapPt, scaleFactor, li);
		for (int i=0, n=ss->size(); i<n; ++i) {
			addSnappedNode(hotPixel, ss, i);
		}
	}
}

/* public static */
bool
SimpleSnapRounder::addSnappedNode(HotPixel& hotPix, SegmentString* segStr, int segIndex)
{
	const Coordinate& p0 = segStr->getCoordinate(segIndex);
	const Coordinate& p1 = segStr->getCoordinate(segIndex + 1);

	if (hotPix.intersects(p0, p1)) {
		//cerr<<"snapped: "<<snapPt<<endl;
		//cerr<<"POINT ("<<snapPt.x<<" "<<snapPt.y<<")"<<endl;
		segStr->addIntersection(hotPix.getCoordinate(), segIndex);

		return true;
	}

	return false;
}

/*private*/
void
SimpleSnapRounder::computeVertexSnaps(SegmentString* e0, SegmentString* e1)
{
	const CoordinateSequence* pts0 = e0->getCoordinatesRO();
	const CoordinateSequence* pts1 = e1->getCoordinatesRO();

	for (unsigned int i0=0, n0=pts0->getSize()-1; i0<n0; i0++)
	{
		const Coordinate& p0 = pts0->getAt(i0);

		HotPixel hotPixel(p0, scaleFactor, li);
		for (unsigned int i1=1, n1=pts1->getSize()-1; i1<n1; i1++)
		{
        		// don't snap a vertex to itself
			if (i0 == i1 && e0 == e1) {
				continue;
			}
//cerr<<"trying "<<p0<<" against "<<pts1->getAt(i1)<<" "<<pts1->getAt(i1 + 1)<<endl;
			bool isNodeAdded = addSnappedNode(hotPixel, e1, i1);
			// if a node is created for a vertex, that vertex must be noded too
			if (isNodeAdded) {
				e0->addIntersection(p0, i0);
			}
		}
	}
 
}

/*public*/
void
SimpleSnapRounder::computeVertexSnaps(const SegmentString::NonConstVect& edges)
{
	for (SegmentString::NonConstVect::const_iterator
			i0=edges.begin(), i0End=edges.end();
			i0!=i0End; ++i0)
	{
		SegmentString* edge0 = *i0;
		for (SegmentString::NonConstVect::const_iterator
				i1=edges.begin(), i1End=edges.end();
				i1!=i1End; ++i1)
		{
			SegmentString* edge1 = *i1;
			computeVertexSnaps(edge0, edge1);
		}
	}
}


/*private*/
void
SimpleSnapRounder::snapRound(SegmentString::NonConstVect* segStrings,
		LineIntersector& li)
{
//TODO: finish me
	vector<Coordinate> intersections;
	findInteriorIntersections(*segStrings, li, intersections);
	computeSnaps(*segStrings, intersections);
	computeVertexSnaps(*segStrings);
}

/*private*/
void
SimpleSnapRounder::findInteriorIntersections(SegmentString::NonConstVect& segStrings,
	LineIntersector& li, vector<Coordinate>& ret)
{
	IntersectionFinderAdder intFinderAdder(li, ret);
	MCIndexNoder noder;
	noder.setSegmentIntersector(&intFinderAdder);
	noder.computeNodes(&segStrings);
	intFinderAdder.getInteriorIntersections();
}


} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.2  2006/02/15 17:19:18  strk
 * NodingValidator synced with JTS-1.7, added CoordinateSequence::operator[]
 * and size() to easy port maintainance.
 *
 * Revision 1.1  2006/02/14 13:28:26  strk
 * New SnapRounding code ported from JTS-1.7 (not complete yet).
 * Buffer op optimized by using new snaprounding code.
 * Leaks fixed in XMLTester.
 *
 **********************************************************************/