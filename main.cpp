#include <Siv3D.hpp>
constexpr int32 gameTime = 60;
constexpr int defaultVibration = 7;

struct Particle {
  Vec2 start;

  Vec2 velocity;
};

struct Spark : IEffect {
  Array<Particle> m_particles;

  explicit Spark(const Vec2 &start)
      : m_particles(50) {
    for (auto &particle : m_particles) {
      particle.start = start + RandomVec2(10.0);

      particle.velocity = Vec2{Random<int>(-50, 50), Random<int>(-50, 50)} * Random(10.0);
    }
  }

  bool update(double t) override {
    for (const auto &particle : m_particles) {
      const Vec2 pos = particle.start + particle.velocity * t + 0.5 * t * t * Vec2{0, 240};

      Triangle{pos, 16.0, (pos.x * 5_deg)}.draw(HSV{pos.y - 40, (1.0 - t)});
    }

    return (t < 1.0);
  }
};

bool isPrime(int64 n) {
  if(n == 1)
      return false;
  for (int64 i = 2; i * i <= n; i++)
    if (!(n % i))
      return false;
  return true;
}

int64 pow10(int n) {
  int64 ret = 1;
  for (int i = 0; i < n; i++)
    ret *= 10;
  return ret;
}

void nextPrime(int64 &prime, int diff) {
  auto p = Random<int>(0, 10);
  auto grothendieck = Random<int>(0, 1000);
  if (grothendieck <= 5) {
    prime = 57;
    return;
  }

  if (diff == 0)
    prime = Random<int64>(0, 50);
  if (diff == 1)
    prime = Random<int64>(0, 100);
  if (diff == 2)
    prime = Random<int64>(100, 1000);
  if (diff == 3)
    prime = Random<int64>(1000, 10000);

  if (p <= 4)
    while (!isPrime(++prime))
      ;
  else
    while (isPrime(++prime) || (2 < diff && !(prime & 1)))
      ;
}

void Main() {
  Window::SetStyle(WindowStyle::Sizable);
  Font font(Scene::Size().x / 10, Typeface::Bold);
  const Array<Audio> audioCorrect = {Audio(U"./Quiz-Correct_Answer01-1.mp3"), Audio(U"./special_correct.m4a")};
  const Array<Audio> audioWrong = {Audio(U"./Quiz-Wrong_Buzzer02-2.mp3"), Audio(U"./special_wrong.m4a")};
  const Array<StringView> diffs = {U"EASY", U"NORMAL", U"HARD", U"INSANE"};

  int64 prime = 2;
  int diff = 1, score = 0;
  int32 leftTime = gameTime;
  bool special = false;
  int32 onePrimeTime = 3;
  int32 fontSize = 10;

  Effect effect;

  while (System::Update()) {
    // init
  INIT:
    ClearPrint();
    special = false;
    while (System::Update()) {
      fontSize = Scene::Size().x / 30;

      bool flag = false;
      for (size_t i = 1; i <= diffs.size(); i++) {
        font(diffs[i - 1]).draw(fontSize, Arg::center(Scene::Size().x / (diffs.size() + 1) * i, Scene::Size().y / 2));
        if (Input(InputDeviceType::Keyboard, 0x30 + i).down()) {
          diff = i - 1;
          flag = true;
          break;
        }
      }

      if (Key0.down())
        special = true;

      if (flag)
        break;
    }
    Stopwatch stopwatch{StartImmediately::Yes};
    score = 0;
    if (diff < 2)
      onePrimeTime = 3;
    else if (diff < 4)
      onePrimeTime = 3;
    leftTime = gameTime;
    nextPrime(prime, diff);

    // game
    while (System::Update()) {
      Stopwatch tmp{StartImmediately::Yes};
      double leftVibration = -1;
      int vibration = defaultVibration;

      while (System::Update() && tmp.s() < onePrimeTime) {

        if (SimpleGUI::Button(U"End Game", Vec2{Scene::Size().x - 150, 10})||Key0.pressed()){
          goto INIT;
        }

        ClearPrint();
        Print << leftTime - stopwatch.s();

        double progress = (double)(leftTime - stopwatch.sF()) / gameTime;
        Rect(0, 0, progress * Scene::Size().x, 10).draw(HSV{120 - 120 * (1. - progress), 0.6, 0.8});
        fontSize = Scene::Size().x / ((diff + 10) / 2);
        double animeSize = sin(stopwatch.sF() * 5) * vibration;
        Circle(Arg::center(Scene::Size().x / 2, Scene::Size().y / 3), fontSize / 1.2).drawArc(tmp.sF() * (360_deg / onePrimeTime), 360_deg - tmp.sF() * (360_deg / onePrimeTime), 0, Scene::Size().x / 80., HSV{120 - tmp.sF() * 40, 0.8, 0.7});
        font(prime).draw(fontSize + animeSize, Arg::center(Scene::Size().x / 2, Scene::Size().y / 3));
        font(U"score").draw((fontSize - animeSize) / 5, Arg::center(Scene::Size().x / 2, Scene::Size().y / 2. + fontSize / 2.));
        font(score).draw(fontSize / 2., Arg::center(Scene::Size().x / 2, Scene::Size().y / 2. + fontSize - animeSize));

        if (KeyEnter.down()) {
          if (isPrime(prime)) {
            effect.add<Spark>(Vec2{Scene::Size().x / 2, Scene::Size().y / 3});
            audioCorrect[special].playOneShot();
            score++;
            break;
          } else {
            audioWrong[special].playOneShot();
            vibration += defaultVibration * 10;
            leftVibration = tmp.sF();
            score--;
          }
        }

        effect.update();

        if (1 <= tmp.sF() - leftVibration)
          vibration = defaultVibration;
        else
          vibration = (vibration * 9 + defaultVibration) / 10;

        if (leftTime - stopwatch.s() < 0)
          break;
      }

      nextPrime(prime, diff);

      if (leftTime - stopwatch.s() < 0)
        break;
    }

    // result manu
    ClearPrint();
    while (System::Update()) {
      font(U"あなたのスコアは", score, U"!!").draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 4));
      font(diffs[diff], U"モード").draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 2));

      if (SimpleGUI::Button(U"End", Vec2{Scene::Size().x / 3, Scene::Size().y / 4 * 3}))
        System::Exit();
      if (SimpleGUI::Button(U"Next Game", Vec2{Scene::Size().x / 3 * 2, Scene::Size().y / 4 * 3}))
        break;
    }
  }
}
