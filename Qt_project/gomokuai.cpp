#include "gomokuai.h"
#include <ctime>
#include <cstdlib>

extern const int boardBlockNumber = 14;//棋格的数目，横竖相同

GomokuAI::GomokuAI()
{
    //初始化棋盘状态
    //所有棋位都没有棋子，用0表示
    for(int i = 0; i != boardBlockNumber + 1; ++i)
    {
        vector<int> boardLine(boardBlockNumber + 1, 0);
        board.push_back(boardLine);
    }

    //人类玩家先手，应该下黑棋
    playerFlag = true;
    firstStepOfAI = true;

    lastChessCol = -1;
    lastChessRow = -1;
    gameStatus = 0;
}



//更新row行，col列的棋位的状态
void GomokuAI::updateBoard(int row, int col)
{
    lastChessCol = col;
    lastChessRow = row;

    if(playerFlag)
    {
        //人类玩家下的棋
        //黑子
        board[row][col] = 1;
    }
    else
    {
        //AI玩家下的棋
        //白子
        board[row][col] = -1;
    }
    updateGameStatus();
}






//轮到AI下棋时
//AI根据当前形势下一个棋子，并更新棋盘状态
void GomokuAI::chessOneByAI()
{
    //AI将要下的棋子的位置
    int row = 0;
    int col = 0;

    if(firstStepOfAI)
    {
        firstStepOfAI = false;
        //如果是AI下的第一步棋，不能按照通用的规则来评分
        //第一步棋应当尽量靠近天元
        if(lastChessRow == 7 && lastChessCol == 7)
        {
            //人类玩家先点了天元
            //此时AI应该在天元附近的8个位置直指或者斜指
            srand((unsigned)time(0));
            int bias1 = 0;
            int bias2 = 0;
            while(bias1 == 0 && bias2 == 0)
            {
                bias1 = rand() % 3 - 1;
                bias2 = rand() % 3 - 1;
            }
            row = 7 + bias1;
            col = 7 + bias2;
        }
        else
        {
            row = 7;
            col = 7;
        }

    }
    else
    {
        int maxScore = -1000000;//AI下一个棋子的最好评分，初始化时给一个很低的评分

        int score = 0;//AI每一次的实际评分
        int AIScore = 0;
        int peopleScore = 0;

        for(int i = 0; i != boardBlockNumber + 1; ++i)
        {
            for(int j = 0; j != boardBlockNumber + 1; ++j)
            {
                if(i == 8 && j == 7)
                {
                    int b = 0;
                }
                if(board[i][j] == 0)
                {
                    //该棋位没有棋子
                    //AI可以落子
                    board[i][j] = -1;

                    //当AI选择了一个落子位置后
                    //人类玩家有很多落子位置
                    //当人类玩家在该种情况下最完美的发挥
                    //应该求出AI的一个最低评分
                    int minScore = 1000000;//AI下完一个棋子后，人类完美发挥，AI得到的评分最低，初始化时给一个很高的评分
                    bool prune = false;//阿尔法剪枝的标志，为true表示进行剪枝，为false则不剪枝
                    for(int k = 0; !prune && k != boardBlockNumber + 1; ++k)
                    {
                        for(int m = 0; !prune && m != boardBlockNumber + 1; ++m)
                        {
                            if(board[k][m] == 0)
                            {
                                if(k == 6 && m == 6)
                                {
                                    int b = 0;
                                }
                                //该棋位没有棋子，人类可以落子
                                board[k][m] = 1;
                                AIScore = calculateScore(i, j);
                                peopleScore = calculateScore(k, m);
                                score = AIScore - peopleScore;

                                //阿尔法剪枝
                                if(score <= maxScore)
                                {
                                    prune = true;
                                }
                                else
                                {
                                    if(score < minScore)
                                    {
                                        minScore = score;
                                    }
                                    else
                                    {
                                        //do nothing
                                    }
                                }

                                //由于所下的两个棋子都是AI幻想出来的
                                //所以现在要将棋盘恢复以前的状态
                                board[k][m] = 0;
                            }
                        }
                    }

                    if(!prune && minScore > maxScore)
                    {
                        maxScore = minScore;
                        row = i;
                        col = j;
                    }
                    else
                    {
                        //do nothing
                    }
                    //由于所下的两个棋子都是AI幻想出来的
                    //所以现在要将棋盘恢复以前的状态
                    board[i][j] = 0;
                }
            }
        }
    }


    //更新棋盘状态
    updateBoard(row, col);
}


int GomokuAI::calculateScore(int row, int col)
{
    int score = 0;


    int chessNum = 1;//相连的同色棋子的数目
    int emptyNum1 = 0;//相连棋子前方的空棋位的个数
    int emptyNum2 = 0;//相连棋子后方的空棋位的个数

    //以当前落子位置为中心
    //沿周围水平、竖直、左上右下、左下右上四个方向查找
    //当在该棋子基础上可能会赢，即五个棋位内没有敌方棋子时，才计分


    for(int x = -1; x != 2; ++x)
    {
        for(int y = 0; y != 2; ++y)
        {
            //-----------------------------通过循环对水平、竖直、左上右下、左下右上四个方向进行评分，并累加---------------------------------//
            //在计算某个方向的评分时，先清零这些计数器
            chessNum = 1;
            emptyNum1 = 0;
            emptyNum2 = 0;

            if(!(x == 0 && y == 0) && !(x == 1 && y == 0))
            {
                //沿既定方向找一遍,扩展四个棋位
                for(int i = 1; i != 5; ++i)
                {
                    if(row + x * i >= 0 &&
                            row + x * i <= boardBlockNumber &&
                            col + y * i >= 0 &&
                            col + y * i <= boardBlockNumber)//判断该棋位是否位于棋盘内
                    {
                        if(board[row + x * i][col + y * i] == board[row][col])//同色
                        {
                            if(emptyNum1 == 0)
                            {
                                ++chessNum;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else if(board[row + x * i][col + y * i] == 0)
                        {
                            //遇到了空棋位
                            ++emptyNum1;
                        }
                        else
                        {
                            //碰到了敌方棋子，停止该方向的寻找
                            break;
                        }
                    }
                }


                //沿相反方向再找一遍，也扩展四个棋位
                for(int i = 1; i != 5; ++i)
                {
                    if(row - x * i >= 0 &&
                            row - x * i <= boardBlockNumber &&
                            col - y * i >= 0 &&
                            col - y * i <= boardBlockNumber)//判断该棋位是否位于棋盘内
                    {
                        if(board[row - x * i][col - y * i] == board[row][col])//同色
                        {
                            if(emptyNum2 == 0)
                            {
                                ++chessNum;
                            }
                            else
                            {
                                break;
                            }
                        }
                        else if(board[row - x * i][col - y * i] == 0)
                        {
                            //遇到了空棋位
                            ++emptyNum2;
                        }
                        else
                        {
                            //碰到了敌方棋子，停止该方向的寻找
                            break;
                        }
                    }
                }


                //-------------------------------算分-------------------------------------//
                if(chessNum + emptyNum1 + emptyNum2 >= 5)
                {
                    //有可能连成五个子，才计分
                    if(chessNum == 1)
                    {
                        if(emptyNum1 == 0 || emptyNum2 == 0)
                        {
                            //死棋
                            //不加分
                        }
                        else
                        {
                            score += 1;
                        }

                    }
                    else if(chessNum == 2)
                    {
                        if(emptyNum1 == 0 || emptyNum2 == 0)
                        {
                            //死棋
                            //不加分
                        }
                        else
                        {
                            score += 5;
                        }
                    }
                    else if(chessNum == 3)
                    {
                        if(emptyNum1 == 0 || emptyNum2 == 0)
                        {
                            //死棋
                            //不加分
                        }
                        else
                        {
                            //活3
                            score += 10;
                        }
                    }
                    else if(chessNum == 4)
                    {
                        if(emptyNum1 == 0 || emptyNum2 == 0)
                        {
                            //冲4
                            if(board[row][col] == 1)
                            {
                                //人类玩家冲4，AI还有挽回的余地
                                score += 10;
                            }
                            else
                            {
                                //AI冲4
                                score += 500;
                            }
                        }
                        else
                        {
                            //活4
                            if(board[row][col] == 1)
                            {
                                //人类玩家活4
                                //除非AI连5、活4、冲4，否则不应该让这种情况出现
                                score += 1000;
                            }
                            else
                            {
                                //AI活4
                                //下一步必胜
                                score += 3000;
                            }
                        }
                    }
                    else//chessNum >= 5
                    {
                        //这种情况必胜
                        if(board[row][col] == 1)
                        {
                            score += 6000;
                        }
                        else
                        {
                            score += 10000;
                        }

                    }
                }
                else
                {
                    //不可能五个子连成一条线
                }
                //-------------------------------算分-------------------------------------//
            }
            else
            {
                //do nothing
            }
            //-----------------------------通过循环对水平、竖直、左上右下、左下右上四个方向进行评分，并累加---------------------------------/
        }
    }
    return score;
}





void GomokuAI::updateGameStatus()
{
    //--------------------------------------------判断输赢--------------------------------------------------//
    //分水平、竖直、左上右下、左下右上四种情况
    //检查以上一次所下棋子分别为第1、2、3、4、5个棋子时，是否有5个棋子连成一条线


    //水平情况
    for(int i = 0; i != 5; ++i)
    {
        if(
                lastChessCol - i >= 0 &&
                lastChessCol - i + 4 <= boardBlockNumber &&
                board[lastChessRow][lastChessCol - i] == board[lastChessRow][lastChessCol - i + 1] &&
                board[lastChessRow][lastChessCol - i] == board[lastChessRow][lastChessCol - i + 2] &&
                board[lastChessRow][lastChessCol - i] == board[lastChessRow][lastChessCol - i + 3] &&
                board[lastChessRow][lastChessCol - i] == board[lastChessRow][lastChessCol - i + 4])
        {
            if(playerFlag)
            {
                gameStatus = 1;//人类玩家胜利
            }
            else
            {
                gameStatus = -1;//AI玩家胜利
            }
            return;
        }
            else
        {
            //do nothing
        }
    }


    //竖直情况
    for(int i = 0; i != 5; ++i)
    {
        if(
                lastChessRow - i >= 0 &&
                lastChessRow - i + 4 <= boardBlockNumber &&
                board[lastChessRow - i][lastChessCol] == board[lastChessRow - i + 1][lastChessCol] &&
                board[lastChessRow - i][lastChessCol] == board[lastChessRow - i + 2][lastChessCol] &&
                board[lastChessRow - i][lastChessCol] == board[lastChessRow - i + 3][lastChessCol] &&
                board[lastChessRow - i][lastChessCol] == board[lastChessRow - i + 4][lastChessCol])
        {
            if(playerFlag)
            {
                gameStatus = 1;//人类玩家胜利
            }
            else
            {
                gameStatus = -1;//AI玩家胜利
            }
            return;
        }
            else
        {
            //do nothing
        }
    }

    //左上、右下情况
    for(int i = 0; i != 5; ++i)
    {
        if(
                lastChessRow - i >= 0 &&
                lastChessRow - i + 4 <= boardBlockNumber &&
                lastChessCol - i >= 0 &&
                lastChessCol - i + 4 <= boardBlockNumber &&
                board[lastChessRow - i][lastChessCol - i] == board[lastChessRow - i + 1][lastChessCol - i + 1] &&
                board[lastChessRow - i][lastChessCol - i] == board[lastChessRow - i + 2][lastChessCol - i + 2] &&
                board[lastChessRow - i][lastChessCol - i] == board[lastChessRow - i + 3][lastChessCol - i + 3] &&
                board[lastChessRow - i][lastChessCol - i] == board[lastChessRow - i + 4][lastChessCol - i + 4])
        {
            if(playerFlag)
            {
                gameStatus = 1;//人类玩家胜利
            }
            else
            {
                gameStatus = -1;//AI玩家胜利
            }
            return;
        }
            else
        {
            //do nothing
        }
    }

    //左下、右上情况
    for(int i = 0; i != 5; ++i)
    {
        if(
                lastChessRow + i <= boardBlockNumber &&
                lastChessRow + i - 4 >= 0 &&
                lastChessCol - i >= 0 &&
                lastChessCol - i + 4 <= boardBlockNumber &&
                board[lastChessRow + i][lastChessCol - i] == board[lastChessRow + i - 1][lastChessCol - i + 1] &&
                board[lastChessRow + i][lastChessCol - i] == board[lastChessRow + i - 2][lastChessCol - i + 2] &&
                board[lastChessRow + i][lastChessCol - i] == board[lastChessRow + i - 3][lastChessCol - i + 3] &&
                board[lastChessRow + i][lastChessCol - i] == board[lastChessRow + i - 4][lastChessCol - i + 4])
        {
            if(playerFlag)
            {
                gameStatus = 1;//人类玩家胜利
            }
            else
            {
                gameStatus = -1;//AI玩家胜利
            }
            return;
        }
            else
        {
            //do nothing
        }
    }
    //-----------------------------------------------------------------------------------------------------//



    //------------------------------------------------判断平局-----------------------------------------------//
    //程序运行到此处，说明未分出胜负
    //若棋盘已满，则为平局
    for(int i = 0; i != boardBlockNumber; ++i)
    {
        for(int j = 0; j != boardBlockNumber; ++j)
        {
            if(board[i][j] == 0)
            {
                //有空棋位
                //不是平局
                //换手，游戏继续进行
                playerFlag = !playerFlag;
                return;
            }
            else
            {
                //do nothing
            }
        }
    }
    //若程序运行到此处
    //说明棋盘已满
    //平局
    gameStatus = 2;
    return;
    //-----------------------------------------------------------------------------------------------------//
}
