#ifndef GOMOKUAI_H
#define GOMOKUAI_H
#include <vector>

//extern const int boardBlockNumber;//棋格的数目，横竖相同

using namespace std;

class GomokuAI
{
public:

    //构造函数
    GomokuAI();

    //用一个元素为vector的vector表示棋盘上的每个棋位的状态
    //0表示该棋位没有棋子
    //1表示该棋位的棋子为黑棋
    //-1表示该棋位的棋子为白棋
    vector<vector<int>> board;

    //上一次落子的位置
    //棋盘上没有棋子时，约定行数与列数均为-1
    int lastChessRow, lastChessCol;

    //游戏状态
    //0：游戏进行中
    //1：人类玩家获胜
    //-1：AI玩家获胜
    //2：平局
    //3:正在评估游戏状态
    int gameStatus;

    //更新row行，col列的棋位的状态
    void updateBoard(int row, int col);


    //区分游戏中正在由谁下棋的标志
    //playerFlag为true表示正在由人类玩家下棋
    //playerFlag为false表示正在由AI下棋
    bool playerFlag;
    bool firstStepOfAI;

    //轮到AI下棋时
    //AI根据当前形势下一个棋子，并更新棋盘状态
    void chessOneByAI();


    //根据棋盘落子情况，判断游戏状态
    void updateGameStatus();

    //根据落子位置和棋盘状态，计算评分
    //row：落子位置的行数
    //col：落子位置的列数
    int calculateScore(int row, int col);

};
#endif // GOMOKUAI_H
