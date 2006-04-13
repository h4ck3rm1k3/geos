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
 * Last port: simplify/TopologyPreservingSimplifier.java rev. 1.4 (JTS-1.7)
 *
 **********************************************************************/

#include <geos/simplify/TopologyPreservingSimplifier.h>
#include <geos/simplify/TaggedLinesSimplifier.h>
#include <geos/simplify/LineSegmentIndex.h> // for auto_ptr dtor
#include <geos/simplify/TaggedLineString.h>
#include <geos/simplify/TaggedLineStringSimplifier.h> // for auto_ptr dtor
#include <geos/algorithm/LineIntersector.h> // for auto_ptr dtor
// for LineStringTransformer inheritance
#include <geos/geom/util/GeometryTransformer.h>
// for LineStringMapBuilderFilter inheritance
#include <geos/geom/GeometryComponentFilter.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/LineString.h>
#include <geos/geom/LinearRing.h>
#include <geos/util/IllegalArgumentException.h>

#include <memory> // for auto_ptr
#include <map>
#include <cassert>

#ifndef GEOS_DEBUG
#define GEOS_DEBUG 0
#endif

#ifdef GEOS_DEBUG
#include <iostream>
#endif

using namespace geos::geom;

namespace geos {
namespace simplify { // geos::simplify

typedef std::map<const geom::Geometry*, TaggedLineString* > LinesMap;


namespace { // module-statics

class LineStringTransformer: public geom::util::GeometryTransformer
{

public:

	friend class TopologyPreservingSimplifier;

protected:

	CoordinateSequence::AutoPtr transformCoordinates(
			const CoordinateSequence* coords,
			const Geometry* parent);

private:

	LineStringTransformer(LinesMap& simp);

	LinesMap& linestringMap;

};

/*private*/
LineStringTransformer::LineStringTransformer(LinesMap& nMap)
	:
	linestringMap(nMap)
{
}

/*protected*/
CoordinateSequence::AutoPtr
LineStringTransformer::transformCoordinates(
		const CoordinateSequence* coords,
		const Geometry* parent)
{
	if ( dynamic_cast<const LineString*>(parent) )
	{
		LinesMap::iterator it = linestringMap.find(parent);
		assert( it != linestringMap.end() );
		
		TaggedLineString* taggedLine = it->second;

		return taggedLine->getResultCoordinates();
	}

	// for anything else (e.g. points) just copy the coordinates
	return GeometryTransformer::transformCoordinates(coords, parent);
}

//----------------------------------------------------------------------

/*
 * This class populates the given LineString=>TaggedLineString map
 * with newly created TaggedLineString objects.
 * Users must take care of deleting the map's values (elem.second).
 * Would be nice if auto_ptr<> worked in a container, but it doesn't :(
 *
 */
class LineStringMapBuilderFilter: public geom::GeometryComponentFilter
{

public:

	friend class TopologyPreservingSimplifier;

	void filter_ro(const Geometry* geom);


private:

	LinesMap& linestringMap;

	LineStringMapBuilderFilter(LinesMap& nMap);
};

/*private*/
LineStringMapBuilderFilter::LineStringMapBuilderFilter(LinesMap& nMap)
	:
	linestringMap(nMap)
{
}

/*public*/
void
LineStringMapBuilderFilter::filter_ro(const Geometry* geom)
{
	TaggedLineString* taggedLine;

	if ( const LinearRing* lr =
			dynamic_cast<const LinearRing*>(geom) )
	{
		taggedLine = new TaggedLineString(lr, 4);

	}
	else if ( const LineString* ls = 
			dynamic_cast<const LineString*>(geom) )
	{
		taggedLine = new TaggedLineString(ls, 2);
	}
	else
	{
		return;
	}

	// Duplicated Geometry pointers shouldn't happen
	if ( ! linestringMap.insert(std::make_pair(geom, taggedLine)).second )
	{
		std::cerr << __FILE__ << ":" << __LINE__ 
		     << "Duplicated Geometry components detected"
		     << std::endl;

		delete taggedLine;
	}
}


} // end of module-statics

/*public static*/
std::auto_ptr<geom::Geometry>
TopologyPreservingSimplifier::simplify(
		const geom::Geometry* geom,
		double tolerance)
{
	TopologyPreservingSimplifier tss(geom);
        tss.setDistanceTolerance(tolerance);
	return tss.getResultGeometry();
}

/*public*/
TopologyPreservingSimplifier::TopologyPreservingSimplifier(const Geometry* geom)
	:
	inputGeom(geom),
	lineSimplifier(new TaggedLinesSimplifier())
{
}

/*public*/
void
TopologyPreservingSimplifier::setDistanceTolerance(double d)
{
	using geos::util::IllegalArgumentException;

	if ( d < 0.0 )
		throw IllegalArgumentException("Tolerance must be non-negative");

	lineSimplifier->setDistanceTolerance(d);
}

/*public*/
std::auto_ptr<geom::Geometry> 
TopologyPreservingSimplifier::getResultGeometry()
{
	LinesMap linestringMap;

	std::auto_ptr<geom::Geometry> result;

	try {
		LineStringMapBuilderFilter lsmbf(linestringMap);
		inputGeom->apply_ro(&lsmbf);

		for (LinesMap::iterator
			it=linestringMap.begin(), itEnd=linestringMap.end();
			it != itEnd;
			++it)
		{
			lineSimplifier->simplifyLine(it->second); 
		}

		LineStringTransformer trans(linestringMap);
		result = trans.transform(inputGeom);

	} catch (...) {
		for (LinesMap::iterator
				it = linestringMap.begin(),
				itEnd = linestringMap.end();
				it != itEnd;
				++it)
		{
			delete it->second;
		}

		throw;
	}

	for (LinesMap::iterator
			it = linestringMap.begin(),
			itEnd = linestringMap.end();
			it != itEnd;
			++it)
	{
		delete it->second;
	}

	return result;
}

} // namespace geos::simplify
} // namespace geos

/**********************************************************************
 * $Log$
 * Revision 1.3  2006/04/13 16:04:10  strk
 * Made TopologyPreservingSimplifier implementation successfully build
 *
 * Revision 1.2  2006/04/13 14:25:17  strk
 * TopologyPreservingSimplifier initial port
 *
 **********************************************************************/