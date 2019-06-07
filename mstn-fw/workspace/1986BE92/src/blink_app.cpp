#include "blink_app.hpp"
#include "task.hpp"

LedDriver Led;

class LedTask: public Task
{
public:
	LedTask(int16_t led, int16_t period) :
			Task("LedTask"),
			led(led),
			period(period)
	{
	}

private:
	int16_t led;
	int16_t period;

	virtual void Execute()
	{
		while (true) {
			Led.Toggle(led);
			Delay(period);
		}
	}
};

void BlinkApp::Initialize()
{
	for (int16_t led = 0; led < Led.GetNum(); led++) {
		int16_t period = (led + 1) * 300 + 200;
		Task::Add(new LedTask(led, period), Task::PriorityNormal, 0x100);
	}
}
