#include <Siv3D.hpp>
constexpr int32 gameTime = 60;
constexpr int32 onePrimeTime = 3;
constexpr int defaultVibration = 7;

bool isPrime(int64 n) {
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
  prime = Random<int64>(pow10(diff), pow10(diff + 1));
  if (grothendieck <= 5) {
    prime = 57;
    return;
  }
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
  Font diffFont(Scene::Size().x / 30);
  const Audio audioCorrect(U"./Quiz-Correct_Answer01-1.mp3");
  const Audio audioWrong(U"./Quiz-Wrong_Buzzer02-2.mp3");
  const Array<StringView> diffs = {U"EASY", U"NORMAL", U"HARD", U"INSANE"};

  int64 prime = 2;
  int diff = 1, score = 0;
  int32 leftTime = gameTime;

  while (System::Update()) {
    // init
    while (System::Update()) {
      if (Scene::Size().x / 30 != diffFont.fontSize())
        diffFont = Font(Scene::Size().x / 30);

      bool flag = false;
      for (size_t i = 1; i <= diffs.size(); i++) {
        diffFont(diffs[i - 1]).draw(Arg::center(Scene::Size().x / (diffs.size() + 1) * i, Scene::Size().y / 2));
        if (Input(InputDeviceType::Keyboard, 0x30 + i).down()) {
          diff = i;
          flag = true;
          break;
        }
      }
      if (flag)
        break;
    }
    Stopwatch stopwatch{StartImmediately::Yes};
    score = 0;
    leftTime = gameTime;
    nextPrime(prime, diff);

    // game
    while (System::Update()) {
      if (Scene::Size().x / 10 != font.fontSize())
        font = Font(Scene::Size().x / 10, Typeface::Bold);

      Stopwatch tmp{StartImmediately::Yes};
      double leftVibration = -1;
      int vibration = defaultVibration;

      while (System::Update() && tmp.s() < onePrimeTime) {
        ClearPrint();
        Print << leftTime - stopwatch.s();

        double progress = (double)(leftTime - stopwatch.sF()) / gameTime;
        Rect(0, 0, progress * Scene::Size().x, 10).draw(HSV{120 - 120 * (1. - progress), 0.6, 0.8});
        int baseSize = Scene::Size().x / (diff * 2 + 3);
        double animeSize = sin(stopwatch.sF() * 5) * vibration;
        Circle(Arg::center(Scene::Size().x / 2, Scene::Size().y / 3), baseSize / 1.2).drawArc(tmp.sF() * (360_deg / onePrimeTime), 360_deg - tmp.sF() * (360_deg / onePrimeTime), 0, Scene::Size().x / 80., HSV{120 - tmp.sF() * 40, 0.8, 0.7});
        font(prime).draw(baseSize + animeSize, Arg::center(Scene::Size().x / 2, Scene::Size().y / 3));
        font(U"score").draw((baseSize - animeSize) / 5, Arg::center(Scene::Size().x / 2, Scene::Size().y / 2. + baseSize / 2.));
        font(score).draw(baseSize / 2., Arg::center(Scene::Size().x / 2, Scene::Size().y / 2. + baseSize - animeSize));

        if (KeyEnter.down()) {
          if (isPrime(prime)) {
            audioCorrect.playOneShot();
            score++;
            break;
          } else {
            audioWrong.playOneShot();
            vibration += defaultVibration * 10;
            leftVibration = tmp.sF();
            score--;
          }
        }

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
      if (Scene::Size().x / 20 != font.fontSize())
        font = Font(Scene::Size().x / 20);

      font(U"あなたのスコアは", score, U"!!").draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 4));
      font(diffs[diff - 1], U"モード").draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 2));

      if (SimpleGUI::Button(U"End", Vec2{Scene::Size().x / 3, Scene::Size().y / 4 * 3}))
        System::Exit();
      if (SimpleGUI::Button(U"Next Game", Vec2{Scene::Size().x / 3 * 2, Scene::Size().y / 4 * 3}))
        break;
    }
  }
}
