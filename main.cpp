#include <Siv3D.hpp>

bool isPrime(int64 n)
{
  for (int64 i = 2; i * i <= n; i++)
    if (!(n % i))
      return false;
  return true;
}

int64 pow10(int n)
{
  int64 ret = 1;
  for (int i = 0; i < n; i++)
    ret *= 10;
  return ret;
}

void nextPrime(int64 &prime, int diff)
{
  auto p = Random<int>(0, 10);
  prime = Random<int64>(pow10(diff), pow10(diff + 1));
  if (p <= 3)
    while (!isPrime(++prime))
      ;
  else
    while (isPrime(++prime) || (2 < diff && !(prime & 1)))
      ;
}

void Main()
{
  Window::SetStyle(WindowStyle::Sizable);
  Font font(Scene::Size().x / 10, Typeface::Bold);
  Font diffFont(Scene::Size().x / 30);
  const Audio audioCorrect(U"./Quiz-Correct_Answer01-1.mp3");
  const Audio audioWrong(U"./Quiz-Wrong_Buzzer02-2.mp3");
  const Array<Audio> BGM = {Audio(U"./easy.m4a"), Audio(U"./easy.m4a"), Audio(U"./easy.m4a"), Audio(U"./hard.m4a")};
  const Array<StringView> diffs = {U"EASY", U"NORMAL", U"HARD", U"INSANE"},
                          results = {U"初心者", U"中級者", U"上級者", U"神"};

  int64 prime = 2;
  int diff = 1, score = 0;
  int32 leftTime = 60;

  while (System::Update())
  {
    // init
    while (System::Update())
    {
      if (Scene::Size().x / 30 != diffFont.fontSize())
        diffFont = Font(Scene::Size().x / 30);

      bool flag = false;
      for (size_t i = 1; i <= diffs.size(); i++)
      {
        diffFont(diffs[i - 1]).draw(Arg::center(Scene::Size().x / (diffs.size() + 1) * i, Scene::Size().y / 2));
        if (Input(InputDeviceType::Keyboard, 0x30 + i).down())
        {
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
    leftTime = 60;
    nextPrime(prime, diff);
    BGM[diff - 1].play(2s);

    // game
    while (System::Update())
    {
      if (Scene::Size().x / 10 != font.fontSize())
        font = Font(Scene::Size().x / 10, Typeface::Bold);

      Stopwatch tmp{StartImmediately::Yes};

      while (System::Update() && tmp.s() < 3)
      {
        ClearPrint();
        Print << leftTime - stopwatch.s();

        double progress = (double)(leftTime - stopwatch.sF()) / 60;
        Rect(0, 0, progress * Scene::Size().x, 10).draw(progress < 0.1 ? Palette::Red : Palette::Green);
        int baseSize = Scene::Size().x / (diff * 2 + 3);
        double animeSize = sin(stopwatch.sF() * 5) * 7;
        font(prime).draw(baseSize + animeSize, Arg::center(Scene::Size().x / 2, Scene::Size().y / 2));

        if (KeyEnter.down())
        {
          if (isPrime(prime))
          {
            audioCorrect.playOneShot();
            nextPrime(prime, diff);
            score++;
            break;
          }
          else
            audioWrong.playOneShot();
        }

        if (leftTime - stopwatch.s() < 0)
          break;
      }
      nextPrime(prime, diff);

      if (leftTime - stopwatch.s() < 0)
        break;
    }
    BGM[diff - 1].stop(2s);

    // result manu
    ClearPrint();
    while (System::Update())
    {
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