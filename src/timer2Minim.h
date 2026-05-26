// мини-класс таймера, версия 2.0
// использован улучшенный алгоритм таймера на millis
// алгоритм чуть медленнее, но обеспечивает кратные интервалы и защиту от пропусков и переполнений

class timerMinim
{
public:
  timerMinim(uint32_t interval);
  void setInterval(uint32_t interval);
  boolean isReady();
  void reset();
  void stop();
  void start();
  boolean isRunning() { return _status; }

  private:
    uint32_t _timer = 0;
    uint32_t _interval = 0;
    bool _status = true;
};

timerMinim::timerMinim(uint32_t interval) {
  _interval = interval;
  _timer = millis();
}

void timerMinim::setInterval(uint32_t interval) {
  _interval = (interval == 0) ? 10 : interval;
}

void timerMinim::start() {
  _status = true;
  _timer = millis();
}

void timerMinim::stop() {
  _status = false;
}

// алгоритм таймера v2.0
boolean timerMinim::isReady() {
  uint32_t thisMls = millis();
  if (_status && thisMls - _timer >= _interval) {
    do {
      _timer += _interval;
      if (_timer < _interval) break;          // переполнение uint32_t
    } while (_timer < thisMls - _interval);  // защита от пропуска шага
    return true;
  } else {
    return false;
  }
}

void timerMinim::reset() {
  _timer = millis();
}
