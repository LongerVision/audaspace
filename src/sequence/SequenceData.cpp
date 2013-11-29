/*******************************************************************************
 * Copyright 2009-2013 Jörg Müller
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include "sequence/SequenceData.h"
#include "sequence/SequenceReader.h"
#include "sequence/SequenceEntry.h"

#include <mutex>

AUD_NAMESPACE_BEGIN

SequenceData::SequenceData(Specs specs, float fps, bool muted) :
	m_specs(specs),
	m_status(0),
	m_entry_status(0),
	m_id(0),
	m_muted(muted),
	m_fps(fps),
	m_speed_of_sound(434),
	m_doppler_factor(1),
	m_distance_model(DISTANCE_MODEL_INVERSE_CLAMPED),
	m_location(3),
	m_orientation(4)
{
	Quaternion q;
	m_orientation.write(q.get());
	float f = 1;
	m_volume.write(&f);

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&m_mutex, &attr);

	pthread_mutexattr_destroy(&attr);
}

SequenceData::~SequenceData()
{
	pthread_mutex_destroy(&m_mutex);
}

void SequenceData::lock()
{
	pthread_mutex_lock(&m_mutex);
}

void SequenceData::unlock()
{
	pthread_mutex_unlock(&m_mutex);
}

void SequenceData::setSpecs(Specs specs)
{
	std::lock_guard<ILockable> lock(*this);

	m_specs = specs;
	m_status++;
}

void SequenceData::setFPS(float fps)
{
	std::lock_guard<ILockable> lock(*this);

	m_fps = fps;
}

void SequenceData::mute(bool muted)
{
	std::lock_guard<ILockable> lock(*this);

	m_muted = muted;
}

bool SequenceData::getMute() const
{
	return m_muted;
}

float SequenceData::getSpeedOfSound() const
{
	return m_speed_of_sound;
}

void SequenceData::setSpeedOfSound(float speed)
{
	std::lock_guard<ILockable> lock(*this);

	m_speed_of_sound = speed;
	m_status++;
}

float SequenceData::getDopplerFactor() const
{
	return m_doppler_factor;
}

void SequenceData::setDopplerFactor(float factor)
{
	std::lock_guard<ILockable> lock(*this);

	m_doppler_factor = factor;
	m_status++;
}

DistanceModel SequenceData::getDistanceModel() const
{
	return m_distance_model;
}

void SequenceData::setDistanceModel(DistanceModel model)
{
	std::lock_guard<ILockable> lock(*this);

	m_distance_model = model;
	m_status++;
}

AnimateableProperty* SequenceData::getAnimProperty(AnimateablePropertyType type)
{
	switch(type)
	{
	case AP_VOLUME:
		return &m_volume;
	case AP_LOCATION:
		return &m_location;
	case AP_ORIENTATION:
		return &m_orientation;
	default:
		return nullptr;
	}
}

std::shared_ptr<SequenceEntry> SequenceData::add(std::shared_ptr<ISound> sound, float begin, float end, float skip)
{
	std::lock_guard<ILockable> lock(*this);

	std::shared_ptr<SequenceEntry> entry = std::shared_ptr<SequenceEntry>(new SequenceEntry(sound, begin, end, skip, m_id++));

	m_entries.push_back(entry);
	m_entry_status++;

	return entry;
}

void SequenceData::remove(std::shared_ptr<SequenceEntry> entry)
{
	std::lock_guard<ILockable> lock(*this);

	m_entries.remove(entry);
	m_entry_status++;
}

AUD_NAMESPACE_END
