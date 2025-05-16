#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include <queue>
#include <vector>

#include "ai.h"
const int BOARD_SIZE = 19;
const int WINDOW_SIZE = 760;  // 40px * 18间距 + 边距
const int MARGIN = 20;
const int CELL_SIZE = (WINDOW_SIZE - 2 * MARGIN) / (BOARD_SIZE - 1);
const float STONE_RADIUS = CELL_SIZE / 2.5f;

enum Stone
{
  Empty,
  Black,
  White
};

// 用于判断气和找连通块
bool hasLiberty(int x, int y, const std::vector<std::vector<Stone>>& board,
                Stone color, std::vector<std::vector<bool>>& visited)
{
  std::queue<std::pair<int, int>> q;
  q.push({x, y});
  visited[y][x] = true;

  const int dx[4] = {1, -1, 0, 0};
  const int dy[4] = {0, 0, 1, -1};

  while (!q.empty())
  {
    auto [cx, cy] = q.front();
    q.pop();
    for (int dir = 0; dir < 4; dir++)
    {
      int nx = cx + dx[dir];
      int ny = cy + dy[dir];
      if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) continue;
      if (board[ny][nx] == Empty) return true;  // 有气
      if (!visited[ny][nx] && board[ny][nx] == color)
      {
        visited[ny][nx] = true;
        q.push({nx, ny});
      }
    }
  }
  return false;
}

// 移除无气的连通块，返回被吃子数
int removeDeadGroup(int x, int y, std::vector<std::vector<Stone>>& board,
                    Stone color)
{
  int removed = 0;
  std::queue<std::pair<int, int>> q;
  std::vector<std::vector<bool>> visited(BOARD_SIZE,
                                         std::vector<bool>(BOARD_SIZE, false));
  q.push({x, y});
  visited[y][x] = true;

  const int dx[4] = {1, -1, 0, 0};
  const int dy[4] = {0, 0, 1, -1};

  std::vector<std::pair<int, int>> group;

  while (!q.empty())
  {
    auto [cx, cy] = q.front();
    q.pop();
    group.push_back({cx, cy});
    for (int dir = 0; dir < 4; dir++)
    {
      int nx = cx + dx[dir];
      int ny = cy + dy[dir];
      if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) continue;
      if (!visited[ny][nx] && board[ny][nx] == color)
      {
        visited[ny][nx] = true;
        q.push({nx, ny});
      }
    }
  }

  for (auto& pos : group)
  {
    board[pos.second][pos.first] = Empty;
    removed++;
  }
  return removed;
}

int main()
{
  sf::RenderWindow window(sf::VideoMode(WINDOW_SIZE, WINDOW_SIZE), "Simple Go",
                          sf::Style::Titlebar | sf::Style::Close);

  std::vector<std::vector<Stone>> board(BOARD_SIZE,
                                        std::vector<Stone>(BOARD_SIZE, Empty));
  bool blackTurn = true;

  int blackCaptured = 0;  // 黑方吃子数
  int whiteCaptured = 0;  // 白方吃子数
  AIWrapper::init();
  sf::SoundBuffer buffer;
  sf::Sound sound;
    bool go_sleep=false;
    if (!buffer.loadFromFile("move.mp3")) {
        return 1;
    }else{
        sound.setBuffer(buffer);
    }
  while (window.isOpen())
  {
    sf::Event event;
    while (window.pollEvent(event))
    {
      if (event.type == sf::Event::Closed) window.close();
      if (!blackTurn)
    {
  auto [aiX, aiY] = AIWrapper::ai_move();
  

  if (board[aiY][aiX] == Empty)
  {
    Stone current = White;
    Stone opponent = Black;
    board[aiY][aiX] = current;

    // 检查四邻点是否是对方棋子，如果无气则吃掉
    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};
    for (int dir = 0; dir < 4; dir++)
    {
      int nx = aiX + dx[dir];
      int ny = aiY + dy[dir];
      if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
        continue;
      if (board[ny][nx] == opponent)
      {
        std::vector<std::vector<bool>> visited(
            BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
        if (!hasLiberty(nx, ny, board, opponent, visited))
        {
          int removed = removeDeadGroup(nx, ny, board, opponent);
          whiteCaptured += removed;
        }
      }
    }

    // 检查自己是否是无气棋子（打劫或自杀）
    std::vector<std::vector<bool>> visited(
        BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
    if (!hasLiberty(aiX, aiY, board, current, visited))
    {
      board[aiY][aiX] = Empty;  // 悔棋
    }
    else
    {
        sound.play();
      
    }
    
  }
  blackTurn = true;
}


      if (event.type == sf::Event::MouseButtonPressed &&
          event.mouseButton.button == sf::Mouse::Left&&blackTurn)
      {
        int mx = event.mouseButton.x;
        int my = event.mouseButton.y;

        int x = (mx - MARGIN + CELL_SIZE / 2) / CELL_SIZE;
        int y = (my - MARGIN + CELL_SIZE / 2) / CELL_SIZE;

        if (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE &&
            board[y][x] == Empty)
        {
          Stone current = blackTurn ? Black : White;
          Stone opponent = blackTurn ? White : Black;

          board[y][x] = current;
            AIWrapper::player_move(x,y);
            
            go_sleep=true;
          // 检查四邻点是否是对方棋子，如果无气则吃掉
          const int dx[4] = {1, -1, 0, 0};
          const int dy[4] = {0, 0, 1, -1};
          for (int dir = 0; dir < 4; dir++)
          {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE)
              continue;
            if (board[ny][nx] == opponent)
            {
              std::vector<std::vector<bool>> visited(
                  BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
              if (!hasLiberty(nx, ny, board, opponent, visited))
              {
                int removed = removeDeadGroup(nx, ny, board, opponent);
                if (current == Black)
                  blackCaptured += removed;
                else
                  whiteCaptured += removed;
              }
            }
          }

          // 同时检查自己棋子有没有无气（打劫或者自杀禁着）
          std::vector<std::vector<bool>> visited(
              BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
          if (!hasLiberty(x, y, board, current, visited))
          {
            // 没气，不能下，悔棋
            board[y][x] = Empty;
          }
          else
          {
            blackTurn = !blackTurn;
            sound.play();
          }
        }
      }
    }

    window.clear(sf::Color(240, 180, 100));  // 棋盘木纹色

    sf::VertexArray lines(sf::Lines);
    for (int i = 0; i < BOARD_SIZE; i++)
    {
      lines.append(sf::Vertex(sf::Vector2f(MARGIN, MARGIN + i * CELL_SIZE),
                              sf::Color::Black));
      lines.append(
          sf::Vertex(sf::Vector2f(WINDOW_SIZE - MARGIN, MARGIN + i * CELL_SIZE),
                     sf::Color::Black));
      lines.append(sf::Vertex(sf::Vector2f(MARGIN + i * CELL_SIZE, MARGIN),
                              sf::Color::Black));
      lines.append(
          sf::Vertex(sf::Vector2f(MARGIN + i * CELL_SIZE, WINDOW_SIZE - MARGIN),
                     sf::Color::Black));
    }
    window.draw(lines);

    int starPoints[] = {3, 9, 15};
    sf::CircleShape star(5);
    star.setFillColor(sf::Color::Black);
    for (int i : starPoints)
    {
      for (int j : starPoints)
      {
        star.setPosition(MARGIN + i * CELL_SIZE - star.getRadius(),
                         MARGIN + j * CELL_SIZE - star.getRadius());
        window.draw(star);
      }
    }

    for (int y = 0; y < BOARD_SIZE; y++)
    {
      for (int x = 0; x < BOARD_SIZE; x++)
      {
        if (board[y][x] != Empty)
        {
          sf::CircleShape stone(STONE_RADIUS);
          stone.setPosition(MARGIN + x * CELL_SIZE - STONE_RADIUS,
                            MARGIN + y * CELL_SIZE - STONE_RADIUS);
          stone.setFillColor(board[y][x] == Black ? sf::Color::Black
                                                  : sf::Color::White);
          stone.setOutlineThickness(1);
          stone.setOutlineColor(sf::Color::Black);
          window.draw(stone);
        }
      }
    }

    // 显示吃子统计
    sf::Font font;
    if (!font.loadFromFile("Ubuntu.ttf"))
    {
      // 如果没有字体文件，文字不会显示
    }
    else
    {
      sf::Text text;
      text.setFont(font);
      text.setCharacterSize(18);
      text.setFillColor(sf::Color::Black);
      text.setPosition(10, WINDOW_SIZE - 40);
      text.setString("Black captured: " + std::to_string(blackCaptured) +
                     "    White captured: " + std::to_string(whiteCaptured));
      window.draw(text);
    }
    
    window.display();
    if (go_sleep){
        go_sleep=false;
        sf::sleep(sf::milliseconds(500));
    }
  }
  while (sound.getStatus() == sf::Sound::Playing) {
        sf::sleep(sf::milliseconds(100));
    }

  return 0;
}
