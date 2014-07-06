/*!
 * \file UserMeasurement.cpp
 * \author Danilo Treffiletti <urban82@gmail.com>
 * \date 2014-07-05
 * \brief Implementation for the UserMeasurement class
 * \copyright 2014 (c) Danilo Treffiletti
 *
 *    This file is part of BeurerScaleManager.
 *
 *    BeurerScaleManager is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    BeurerScaleManager is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with BeurerScaleManager.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UserMeasurement.hpp"

UserMeasurement::UserMeasurement(QObject* parent)
    : QObject(parent)
{
}

UserMeasurement::~UserMeasurement()
{
}

QDateTime UserMeasurement::getDateTime() const
{
    return m_dateTime;
}

void UserMeasurement::setDateTime(const QDateTime& dateTime)
{
    m_dateTime = dateTime;
}

double UserMeasurement::getWeight() const
{
    return m_weight;
}

void UserMeasurement::setWeight(const double& weight)
{
    m_weight = weight;
}

double UserMeasurement::getBodyFatPercent() const
{
    return m_bodyFatPercent;
}

void UserMeasurement::setBodyFatPercent(const double& bodyFatPercent)
{
    m_bodyFatPercent = bodyFatPercent;
}

double UserMeasurement::getWaterPercent() const
{
    return m_waterPercent;
}

void UserMeasurement::setWaterPercent(const double& waterPercent)
{
    m_waterPercent = waterPercent;
}

double UserMeasurement::getMusclePercent() const
{
    return m_musclePercent;
}

void UserMeasurement::setMusclePercent(const double& musclePercent)
{
    m_musclePercent = musclePercent;
}