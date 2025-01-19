#pragma once
#include <ctime>
#include <chrono>

#include "core/valueTypes.h"

namespace dxray
{
	/// <summary>
	/// Stopwatch class used to track time, declarations are based on floating/double precision floating values.
	/// The stopwatch can be used as a timer through starting once and resetting at intervals. This won't stop
	/// the ticking, but will reset the timer. If you however want to stop the ticking, call stop.
	/// </summary>
	/// <typeparam name="Precision">Defines the precision of the value type, current specializations are fp32 and fp64.</typeparam>
	template<typename Precision>
	class Stopwatch
	{
		using ValueType = Precision;

	public:
		Stopwatch(bool a_bBeginTicking = false);
		~Stopwatch() = default;

		void Start();
		void Stop();
		void Reset(bool a_bStopTicking = false);

		ValueType GetElapsedNs();
		ValueType GetElapsedMs();
		ValueType GetElapsedSeconds();

	private:
		std::chrono::high_resolution_clock::time_point m_startTime;
		ValueType m_elapsedTime;
		bool m_isTicking;
	};

	template<typename Precision>
	Stopwatch<Precision>::Stopwatch(bool a_bBeginTicking /*= false*/) :
		m_startTime(),
		m_elapsedTime(static_cast<Precision>(0.0)),
		m_isTicking(false)
	{
		if (a_bBeginTicking)
		{
			Start();
		}
	}

	template<typename Precision>
	inline void Stopwatch<Precision>::Start()
	{
		m_startTime = std::chrono::high_resolution_clock::now();
		m_isTicking = true;
	}

	template<typename Precision>
	inline void Stopwatch<Precision>::Stop()
	{
		DXRAY_ASSERT(m_isTicking);
		m_elapsedTime += std::chrono::duration<ValueType, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count();
		m_isTicking = false;
	}

	template<typename Precision>
	inline void Stopwatch<Precision>::Reset(bool a_bStopTicking)
	{
		m_elapsedTime = 0.0f;

		if (a_bStopTicking)
		{
			m_isTicking = false;
			return;
		}

		m_startTime = std::chrono::high_resolution_clock::now();
	}

	template<typename Precision>
	inline Stopwatch<Precision>::ValueType Stopwatch<Precision>::GetElapsedNs()
	{
		return GetElapsedSeconds() * static_cast<ValueType>(1'000'000);
	}

	template<typename Precision>
	inline Stopwatch<Precision>::ValueType Stopwatch<Precision>::GetElapsedMs()
	{
		return GetElapsedSeconds() * static_cast<ValueType>(1'000);
	}

	template<typename Precision>
	inline Stopwatch<Precision>::ValueType Stopwatch<Precision>::GetElapsedSeconds()
	{
		return m_isTicking 
			? std::chrono::duration<ValueType, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - m_startTime).count()
			: m_elapsedTime;
	}

	//Declarations.
	using Stopwatchf = Stopwatch<fp32>;
	using Stopwatchd = Stopwatch<fp64>;
}