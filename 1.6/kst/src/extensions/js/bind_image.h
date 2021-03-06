/***************************************************************************
                                bind_image.h
                             -------------------
    begin                : Nov 15 2007
    copyright            : (C) 2007 The University of British Columbia
    email                :
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef BIND_IMAGE_H
#define BIND_IMAGE_H

#include "bind_dataobject.h"
#include "bind_object.h"

#include <kstimage.h>
#include <kstmatrix.h>

/* @class Image
   @inherits DataObject
   @description An image object created from a matrix.
*/
class KstBindImage : public KstBindDataObject {
  public:
    /* @constructor
       @description Main constructor for Image class.  Constructing a new
                    image automatically adds it to the data collection of Kst.
       @arg Matrix matrix The matrix for the image.  Can be specified as a string
                     containing the tag name of an existing matrix, or as a
                     matrix object.
    */
    KstBindImage(KJS::ExecState *exec, KstImagePtr d);
    KstBindImage(KJS::ExecState *exec, KJS::Object *globalObject = 0L);
    ~KstBindImage();

    KJS::Object construct(KJS::ExecState *exec, const KJS::List& args);
    KJS::Value call(KJS::ExecState *exec, KJS::Object& self, const KJS::List& args);
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    void put(KJS::ExecState *exec, const KJS::Identifier& propertyName, const KJS::Value& value, int attr = KJS::None);
    KJS::ReferenceList propList(KJS::ExecState *exec, bool recursive = true);
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier& propertyName) const;
    int methodCount() const;
    int propertyCount() const;

    /* @method minMaxThreshold
       @returns 
       @description Set the thresholding to the current minimum and maximum of the 
                    associated matrix.
    */
    KJS::Value minMaxThreshold(KJS::ExecState *exec, const KJS::List& args);

    /* @method smartThreshold
       @returns 
       @description Set the thresholding to automatically calculated smart values based
                    on the current values of the the associated matrix.
    */
    KJS::Value smartThreshold(KJS::ExecState *exec, const KJS::List& args);

    /* @property Matrix matrix
       @description The matrix associated with the image.
    */
    void setMatrix(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value matrix(KJS::ExecState *exec) const;

    /* @property number map
       @description The map type of the image.
                    <ul>
                    <li>0 - Color map</li>
                    <li>1 - Contour map</li>
                    <li>2 - Color map and contour map</li>
                    </ul>
    */
    void setMap(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value map(KJS::ExecState *exec) const;

    /* @property string palette
       @description The name of the palette associated with the image color map.
    */    void setPalette(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value palette(KJS::ExecState *exec) const;

    /* @property number lowerThreshold
       @description The lower threshold associated with the map.
    */
    void setLowerThreshold(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value lowerThreshold(KJS::ExecState *exec) const;

    /* @property number upperThreshold
       @description The upper threshold associated with the map.
    */
    void setUpperThreshold(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value upperThreshold(KJS::ExecState *exec) const;

    /* @property boolean autoThreshold
       @description If true, automatic thresholding is enabled.
    */
    void setAutoThreshold(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value autoThreshold(KJS::ExecState *exec) const;

    /* @property number numContours
       @description The number of contour levels if the contour map is enabled.
    */
    void setNumContours(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value numContours(KJS::ExecState *exec) const;

    /* @property number contourWeight
       @description The weight of the lines used to draw the contour map, 
                    if the contour map is enabled. A value of -1 can be
                    used for a variable contour weight.
    */
    void setContourWeight(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value contourWeight(KJS::ExecState *exec) const;

    /* @property string color
       @description The color of the contours.  Stored in the form "#RRGGBB" as
                    hex values.  This property can also be set with English
                    strings such as "blue".
    */
    void setContourColor(KJS::ExecState *exec, const KJS::Value& value);
    KJS::Value contourColor(KJS::ExecState *exec) const;

  protected:
    KstBindImage(int id);
    void addBindings(KJS::ExecState *exec, KJS::Object& obj);
    static KstBindDataObject *bindFactory(KJS::ExecState *exec, KstDataObjectPtr obj);
};

#endif
