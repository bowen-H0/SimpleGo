#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <map>
#include <stack>
#include <utility>
#include <vector>

namespace AIWrapper
{

constexpr int BOARD_SIZE = 19;
enum Cell
{
  EMPTY = 0,
  PLAYER = 1,
  AI = 2
};

static std::vector<std::vector<int>> board;
static std::pair<int, int> last_ko_pos = {-1, -1};  // 劫争位置记录

struct AtariLiberty
{
  std::pair<int, int> pos;
  int group_size;
};

void init()
{
  board = std::vector<std::vector<int>>(BOARD_SIZE,
                                        std::vector<int>(BOARD_SIZE, EMPTY));
  std::srand((unsigned)std::time(nullptr));
  last_ko_pos = {-1, -1};
}

std::vector<std::vector<int>> backup_board;

void save_board()
{
  backup_board = board;
}

void restore_board()
{
  board = backup_board;
}

bool in_bounds(int x, int y)
{
  return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

void dfs_group(int x, int y, int color, std::vector<std::pair<int, int>>& group,
               int& liberties, std::vector<std::vector<bool>>& visited,
               std::vector<std::vector<bool>>& visited_liberties)
{
  static const int dx[] = {1, -1, 0, 0};
  static const int dy[] = {0, 0, 1, -1};

  std::stack<std::pair<int, int>> stack;
  stack.push({x, y});
  visited[x][y] = true;

  while (!stack.empty())
  {
    auto [cx, cy] = stack.top();
    stack.pop();
    group.push_back({cx, cy});

    for (int dir = 0; dir < 4; ++dir)
    {
      int nx = cx + dx[dir], ny = cy + dy[dir];
      if (!in_bounds(nx, ny)) continue;

      if (board[nx][ny] == EMPTY)
      {
        if (!visited_liberties[nx][ny])
        {
          visited_liberties[nx][ny] = true;
          liberties++;
        }
      }
      else if (board[nx][ny] == color && !visited[nx][ny])
      {
        visited[nx][ny] = true;
        stack.push({nx, ny});
      }
    }
  }
}
void remove_dead_groups(int enemy_color)
{
  std::vector<std::vector<bool>> visited(BOARD_SIZE,
                                         std::vector<bool>(BOARD_SIZE, false));
  for (int x = 0; x < BOARD_SIZE; ++x)
  {
    for (int y = 0; y < BOARD_SIZE; ++y)
    {
      if (board[x][y] == enemy_color && !visited[x][y])
      {
        std::vector<std::pair<int, int>> group;
        int liberties = 0;
        std::vector<std::vector<bool>> visited_liberties(
            BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
        dfs_group(x, y, enemy_color, group, liberties, visited,
                  visited_liberties);
        if (liberties == 0)
        {
          for (auto& p : group)
          {
            board[p.first][p.second] = EMPTY;
          }
        }
      }
    }
  }
}
bool is_move_legal(int x, int y, int color)
{
  if (!in_bounds(x, y) || board[x][y] != EMPTY)
  {
    return false;
  }

  // 劫争检查
  if (std::make_pair(x, y) == last_ko_pos)
  {
    return false;
  }

  save_board();
  board[x][y] = color;
  remove_dead_groups(3 - color);  // 移除对方死子

  // 检查自身气数
  std::vector<std::pair<int, int>> group;
  int self_liberties = 0;
  std::vector<std::vector<bool>> visited(BOARD_SIZE,
                                         std::vector<bool>(BOARD_SIZE, false));
  std::vector<std::vector<bool>> visited_liberties(
      BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
  dfs_group(x, y, color, group, self_liberties, visited, visited_liberties);

  // 检查是否形成劫
  if (group.size() == 1 && self_liberties == 1)
  {
    last_ko_pos = {x, y};  // 记录劫争位置
  }

  restore_board();
  return self_liberties > 0;
}

std::vector<AtariLiberty> find_atari_liberties(int target_color)
{
  std::vector<AtariLiberty> liberties;
  std::vector<std::vector<bool>> visited(BOARD_SIZE,
                                         std::vector<bool>(BOARD_SIZE, false));
  static const int dx[] = {1, -1, 0, 0};
  static const int dy[] = {0, 0, 1, -1};

  for (int x = 0; x < BOARD_SIZE; ++x)
  {
    for (int y = 0; y < BOARD_SIZE; ++y)
    {
      if (board[x][y] == target_color && !visited[x][y])
      {
        std::vector<std::pair<int, int>> group;
        int libs = 0;
        std::vector<std::vector<bool>> visited_liberties(
            BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
        dfs_group(x, y, target_color, group, libs, visited, visited_liberties);

        if (libs == 1)
        {
          std::pair<int, int> lib_point = {-1, -1};
          for (auto& p : group)
          {
            for (int d = 0; d < 4; ++d)
            {
              int nx = p.first + dx[d], ny = p.second + dy[d];
              if (in_bounds(nx, ny) && board[nx][ny] == EMPTY)
              {
                lib_point = {nx, ny};
                break;
              }
            }
            if (lib_point.first != -1) break;
          }

          if (lib_point.first != -1)
          {
            bool exists = false;
            for (auto& item : liberties)
            {
              if (item.pos == lib_point)
              {
                if (item.group_size < group.size())
                {
                  item.group_size = group.size();
                }
                exists = true;
                break;
              }
            }
            if (!exists)
            {
              liberties.push_back({lib_point, static_cast<int>(group.size())});
            }
          }
        }
      }
    }
  }
  return liberties;
}

int evaluate_position(int x, int y)
{
  // 基础分
  int center_x = BOARD_SIZE / 2, center_y = BOARD_SIZE / 2;
  int distance = abs(x - center_x) + abs(y - center_y);
  int score = (BOARD_SIZE - distance) * 3;

  // 邻近棋子影响
  int ai_adj = 0, player_adj = 0;
  static const int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
  static const int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};
  for (int d = 0; d < 8; ++d)
  {
    int nx = x + dx[d], ny = y + dy[d];
    if (in_bounds(nx, ny))
    {
      if (board[nx][ny] == AI)
        ai_adj++;
      else if (board[nx][ny] == PLAYER)
        player_adj++;
    }
  }
  score += ai_adj * 5;
  score -= player_adj * 4;  // 增加对敌方邻近的惩罚

  // 气数评估
  int liberty_count = 0;
  std::vector<std::vector<bool>> visited_liberties(
      BOARD_SIZE, std::vector<bool>(BOARD_SIZE, false));
  for (int d = 0; d < 4; ++d)
  {
    int nx = x + dx[d], ny = y + dy[d];
    if (in_bounds(nx, ny) && board[nx][ny] == EMPTY &&
        !visited_liberties[nx][ny])
    {
      liberty_count++;
      visited_liberties[nx][ny] = true;
    }
  }
  score += liberty_count * 6;  // 提高气数权重

  // 避免靠近厚势
  if (player_adj >= 2)
  {
    score -= 15 * player_adj;
  }

  return score;
}
std::pair<int, int> ai_move()
{
  // 阶段1：吃子（优先大群体）
  auto eat_candidates = find_atari_liberties(PLAYER);
  std::sort(eat_candidates.begin(), eat_candidates.end(),
            [](const AtariLiberty& a, const AtariLiberty& b)
            { return a.group_size > b.group_size; });

  for (const auto& candidate : eat_candidates)
  {
    auto [x, y] = candidate.pos;
    if (is_move_legal(x, y, AI) && candidate.pos != last_ko_pos)
    {
      board[x][y] = AI;
      remove_dead_groups(PLAYER);
      last_ko_pos = candidate.pos;
      return {x, y};
    }
  }

  // 阶段2：防御（保护大群体）
  auto save_candidates = find_atari_liberties(AI);
  std::sort(save_candidates.begin(), save_candidates.end(),
            [](const AtariLiberty& a, const AtariLiberty& b)
            { return a.group_size > b.group_size; });

  for (const auto& candidate : save_candidates)
  {
    auto [x, y] = candidate.pos;
    if (is_move_legal(x, y, AI))
    {
      board[x][y] = AI;
      remove_dead_groups(PLAYER);
      return {x, y};
    }
  }

  // 阶段3：战略扩展
  static const int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
  static const int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};

  std::vector<std::pair<int, int>> expand_candidates;
  std::vector<std::vector<bool>> visited(BOARD_SIZE,
                                         std::vector<bool>(BOARD_SIZE, false));

  for (int x = 0; x < BOARD_SIZE; ++x)
  {
    for (int y = 0; y < BOARD_SIZE; ++y)
    {
      if (board[x][y] == EMPTY && !visited[x][y])
      {
        int ai_adj = 0;
        int liberties = 0;

        // 计算气数和连接性
        for (int d = 0; d < 4; ++d)
        {
          int nx = x + dx[d], ny = y + dy[d];
          if (in_bounds(nx, ny))
          {
            if (board[nx][ny] == EMPTY) liberties++;
            if (board[nx][ny] == AI) ai_adj++;
          }
        }

        // 确保至少有3口气且不过于拥挤
        if (liberties >= 3 && ai_adj <= 3)
        {
          expand_candidates.emplace_back(x, y);
          visited[x][y] = true;
        }
      }
    }
  }

  // 按评估值排序
  std::sort(expand_candidates.begin(), expand_candidates.end(),
            [](const auto& a, const auto& b)
            {
              return evaluate_position(a.first, a.second) >
                     evaluate_position(b.first, b.second);
            });

  for (const auto& move : expand_candidates)
  {
    if (is_move_legal(move.first, move.second, AI))
    {
      board[move.first][move.second] = AI;
      remove_dead_groups(PLAYER);
      return move;
    }
  }

  // 阶段4：最终安全落子检查
  std::vector<std::pair<int, int>> fallback_moves;
  for (int x = 0; x < BOARD_SIZE; ++x)
  {
    for (int y = 0; y < BOARD_SIZE; ++y)
    {
      if (board[x][y] == EMPTY && is_move_legal(x, y, AI))
      {
        fallback_moves.emplace_back(x, y);
      }
    }
  }

  if (!fallback_moves.empty())
  {
    std::sort(fallback_moves.begin(), fallback_moves.end(),
              [](const auto& a, const auto& b)
              {
                return evaluate_position(a.first, a.second) >
                       evaluate_position(b.first, b.second);
              });
    auto best = fallback_moves[0];
    board[best.first][best.second] = AI;
    remove_dead_groups(PLAYER);
    return best;
  }

  return {0, 0};
}

void player_move(int x, int y)
{
  if (is_move_legal(x, y, PLAYER))
  {
    board[x][y] = PLAYER;
    remove_dead_groups(AI);
    last_ko_pos = {x, y};  // 记录玩家提子位置
  }
}

}  // namespace AIWrapper