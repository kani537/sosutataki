#include <Siv3D.hpp>

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

void nextPrimes(Array<int64> &primes, int diff) {
  primes.clear();
  auto p = Random<int>(0, (diff + 5) / 2 - 1);
  for (int i = 0; i < (diff + 5) / 2; i++) {
    auto r = Random<int64>(pow10(diff), pow10(diff + 1));
    if (p == i)
      while (!isPrime(++r))
        ;
    else
      while (isPrime(++r) || (2 < diff && !(r & 1)))
        ;
    primes.push_back(r);
  }
}

void Main() {
  Window::SetStyle(WindowStyle::Sizable);
  Font font(Scene::Size().x / 20);
  Font diffFont(Scene::Size().x / 30);
  const Audio audioCorrect(U"./Quiz-Correct_Answer01-1.mp3");
  const Audio audioWrong(U"./Quiz-Wrong_Buzzer02-2.mp3");

  Stopwatch stopwatch{StartImmediately::Yes};
  const Array<StringView> diffs = {U"EASY", U"NORMAL", U"HARD", U"INSANE"},
                          results = {U"初心者", U"中級者", U"上級者", U"神"};

  Array<int64> primes;
  int diff = 2, score = 0;
  int32 leftTime = 60;

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
    score = 0;
    leftTime = 60;
    nextPrimes(primes, diff);

    // game
    while (System::Update()) {
      if (Scene::Size().x / 20 != font.fontSize())
        font = Font(Scene::Size().x / 20);

      ClearPrint();
      Print << leftTime - stopwatch.s();

      for (size_t i = 1; i <= primes.size(); i++)
        font(primes[i - 1]).draw(Arg::center(Scene::Size().x / (primes.size() + 1) * i, Scene::Size().y / 2));

      for (size_t i = 0; i < primes.size(); i++)
        if (Input(InputDeviceType::Keyboard, 0x31 + i).down() && i < primes.size()) {
          if (isPrime(primes[i])) {
            audioCorrect.playOneShot();
            nextPrimes(primes, diff);
            leftTime += 3;
            score++;
            break;
          } else {
            audioWrong.playOneShot();
            leftTime -= 5;
          }
        }

      if (leftTime - stopwatch.s() < 0)
        break;
    }

    // result menu
    ClearPrint();
    while (System::Update()) {
      if (Scene::Size().x / 20 != font.fontSize())
        font = Font(Scene::Size().x / 20);

      font(U"あなたのスコアは", score, U"!!").draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 4));
      font(diffs[diff - 1], U"モード").draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 2));
      font(results[2]).draw(Arg::center(Scene::Size().x / 2, Scene::Size().y / 2 + font.fontSize()));

      if (SimpleGUI::Button(U"End", Vec2{Scene::Size().x / 3, Scene::Size().y / 4 * 3}))
        System::Exit();
      if (SimpleGUI::Button(U"Next Game", Vec2{Scene::Size().x / 3 * 2, Scene::Size().y / 4 * 3}))
        break;
    }
  }
}
