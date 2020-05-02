#pragma once
#include <ArduinoLogger.h>
#include "ring_span.hpp"

static uint32_t _timeAtLoopStart;


/**
 * @brief based on the circular log buffer plugin with some small added features like time management from start isntant
 * 
 * @tparam (1 * 1024) 
 */
template <size_t TBufferSize = (1 * 1024)>
class SanBufferLogger final : public LoggerBase
{
  public:
	/// Default constructor
	  SanBufferLogger() : LoggerBase() {}

	  /** Initialize the circular log buffer with options
	 *
	 * @param enable If true, log statements will be output to the log buffer. If false,
	 * logging will be disabled and log statements will not be output to the log buffer.
	 * @param l Runtime log filtering level. Levels greater than the target will not be output
	 * to the log buffer.
	 * @param echo If true, log statements will be logged and printed to the console with printf().
	 * If false, log statements will only be added to the log buffer.
	 */
	  explicit SanBufferLogger(bool enable, log_level_e l = LOG_LEVEL_LIMIT(),
							   bool echo = LOG_ECHO_EN_DEFAULT) noexcept
		  : LoggerBase(enable, l, echo)
	  {
	}

	/// Default destructor
	~SanBufferLogger() noexcept = default;

	size_t size() const noexcept final
	{
		return log_buffer_.size();
	}

	size_t capacity() const noexcept final
	{
		return log_buffer_.capacity();
	}


	virtual void log_customprefix() override{
		print("%i ", millis() - _timeAtLoopStart);
	}

	void flush() noexcept final
	{
		while(!log_buffer_.empty())
		{
			_putchar(log_buffer_.pop_front());
		}
	}

	void clear() noexcept final
	{
		while(!log_buffer_.empty())
		{
			log_buffer_.pop_front();
		}
	}

	/**
	 * @brief Set the start instant
	 * 
	 */
	static void SetLogTimeStart()
	{
		_timeAtLoopStart =millis();
	}
	

  protected:
	void log_putc(char c) noexcept final
	{
		log_buffer_.push_back(c);
	}


  private:
	char buffer_[TBufferSize] = {0};
	stdext::ring_span<char> log_buffer_{buffer_, buffer_ + TBufferSize};
};

