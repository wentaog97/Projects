#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <set>

using namespace std;

// Pieces
#define EMPTY 0
#define PAWN 1
#define ROOK 2
#define KNIGHT 3
#define BISHOP 4
#define QUEEN 5
#define KING 6
#define PIECE 7
#define BLACK 8
#define WHITE 0

// Castling constants
#define WHITE_KING_SIDE 0
#define WHITE_QUEEN_SIDE 1
#define BLACK_KING_SIDE 2
#define BLACK_QUEEN_SIDE 3
#define WHITE_KING 0
#define BLACK_KING 1

#define TESTING 1

uint8_t board[64] = {0};
bool turn = 0; // White turn = 0, black turn = 1
vector<int> capturedPieces;
bool rookCanCastle[4] = {1,1,1,1}; // White King side, White Queen side, Black King side, Black Queen side
bool kingHasNotMoved[2] = {1,1}; // Black King, White King
int enPassantTile = -1;
int enPassantPawnPosition = -1;

// Forward declarations
int getPiece(int);
int getColor(int);
void displayBoard(uint8_t * = board);
void displayBoardValues(uint8_t * = board);
int initBoard(const char*, uint8_t * = board);
void movePiece(int, int, uint8_t * = board, bool = false);
bool isInCheck(int , uint8_t * = board);
bool canKingCastle(int, uint8_t * = board);
bool addMove(int, int, vector<int> &res, int, uint8_t * = board);
bool addMove(int, int, set<int> &res, int, uint8_t * = board);
bool isInCheckAfterMoving(int, int, uint8_t * = board);
vector<int> checkValidMoves(int , uint8_t * = board);
vector<int> checkAllValidMoves(int , uint8_t * = board);
set<int> checkAllPosBeingAttackedBy(int, uint8_t * = board);
vector<int> filterValidMoves(int, vector<int>, uint8_t * = board);

// Implementations
void displayBoardValues(uint8_t * board){
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            cout << "[" << board[r*8+c] << "] ";
        }
        cout << endl;
    }
}

void displayBoard(uint8_t * board){
    for(int r = 0; r < 8; r++){
        for(int c = 0; c < 8; c++){
            int piece = board[r * 8 + c];
            char pieceCh = ' ';

            switch (piece)
            {
            case 1: pieceCh = 'P'; break;
            case 2: pieceCh = 'R'; break;
            case 3: pieceCh = 'N'; break;
            case 4: pieceCh = 'B'; break;
            case 5: pieceCh = 'Q'; break;
            case 6: pieceCh = 'K'; break;
            case 9: pieceCh = 'p'; break;
            case 10: pieceCh = 'r'; break;
            case 11: pieceCh = 'n'; break;
            case 12: pieceCh = 'b'; break;
            case 13: pieceCh = 'q'; break;
            case 14: pieceCh = 'k'; break;
            default: break;
            }
            cout << "[" << pieceCh << "] ";
        }
        cout << endl;
    }
}

void displayInfo(){
    if(turn == WHITE){
        cout << "White's Turn!" << endl;
    } else {
        cout << "Black's Turn!" << endl;
    }
}

int getPiece(int n){
    return (n & PIECE);
}

int getColor(int n){
    return (n & BLACK) ;
}

bool isRowColValid(int r, int c){
    return r >= 0 && r < 8 && c >= 0 && c < 8;
}

int initBoard(const char* fen, uint8_t * board){
    int i = 0; // index in the board array
    while (*fen && *fen != ' ') {
        if (isdigit(*fen)) {
            i += *fen - '0';
        } else if (*fen == '/') {
            fen++; // Move to the next character after '/'
            continue;
        } else {
            int piece = EMPTY;
            switch(*fen) {
                case 'P': piece = PAWN | WHITE; break;
                case 'R': piece = ROOK | WHITE; break;
                case 'N': piece = KNIGHT | WHITE; break;
                case 'B': piece = BISHOP | WHITE; break;
                case 'Q': piece = QUEEN | WHITE; break;
                case 'K': piece = KING | WHITE; break;
                case 'p': piece = PAWN | BLACK; break;
                case 'r': piece = ROOK | BLACK; break;
                case 'n': piece = KNIGHT | BLACK; break;
                case 'b': piece = BISHOP | BLACK; break;
                case 'q': piece = QUEEN | BLACK; break;
                case 'k': piece = KING | BLACK; break;
                default:
                    cerr << "Invalid character in FEN string." << endl;
                    return -1;
            }
            board[i++] = piece;
        }
        fen++;
    }
    
    return 0;
}

// Assuming input is valid and not being checked
// Testing Mode does not change game state after calling
void movePiece(int ori, int pos, uint8_t * board, bool testingMode){
    if(!board[ori]) {
        cout << "Empty tile!" << endl;
        return;
    }

    int piece = getPiece(board[ori]);
    int color = getColor(board[ori]);

    // If target position is empty, move to empty space
    if(!board[pos]){
        // All pawn logics, including promotion, enpassant, move 2, move 1
        if(piece == PAWN){
            // If it's pawn and reached last row, promote piece
            if((pos >=0 && pos <8) || (pos >=56 && pos <64)){
                board[pos] = (color | QUEEN); // Need to separate this for it's own function
                board[ori] = 0;
                if(!testingMode){
                    enPassantTile = -1;
                    enPassantPawnPosition = -1;
                    turn = !turn;
                }
                return;
            }
            // If it's pawn and attack sqaure can be enpassant
            if(pos == enPassantTile){
                board[pos] = board[ori];
                board[ori] = 0;
                capturedPieces.push_back(board[enPassantPawnPosition]);
                board[enPassantPawnPosition] = 0;
                if(!testingMode){
                    enPassantTile = -1;
                    enPassantPawnPosition = -1;
                    turn = !turn;
                }
                return;
            }

            // If it's pawn and moved 2 squares mark enpassant info
            if(abs(pos-ori)==16){
                board[pos] = board[ori];
                board[ori] = 0;
                if(!testingMode){
                    enPassantTile = ori;
                    enPassantPawnPosition = pos;
                    turn = !turn;
                }
                return;
            }

            // If it's pawn and moved 1 square
            board[pos] = board[ori];
            board[ori] = 0;
            if(!testingMode){
                enPassantTile = -1;
                enPassantPawnPosition = -1;
                turn = !turn;
            }
            return;
        } 

        // Castle move logic
        if(piece == KING && (abs(pos-ori)==2)){
            if((pos-ori)==-2){ // Queen Side Castle
                board[ori-1]=board[ori-4];
                board[ori-4]=0;
            } else { // King Side Castle
                board[ori+1]=board[ori+3];
                board[ori+3]=0;
            }
        }

        // If king moves, remove all castle availability
        if(piece == KING && !testingMode){
            if(color==WHITE){
                rookCanCastle[WHITE_KING_SIDE] = 0;
                rookCanCastle[WHITE_QUEEN_SIDE] = 0;

                kingHasNotMoved[WHITE_KING] = 0;
            } else {
                rookCanCastle[BLACK_KING_SIDE] = 0;
                rookCanCastle[BLACK_QUEEN_SIDE] = 0;

                kingHasNotMoved[BLACK_KING] = 0;
            }
        }

        // If rook moves, remove the castle availability of that side
        if(piece == ROOK && !testingMode){
            if(color==WHITE){
                if(ori == 56) rookCanCastle[WHITE_KING_SIDE] = 0;
                if(ori == 63) rookCanCastle[WHITE_QUEEN_SIDE] = 0;
            } else {
                if(ori == 0) rookCanCastle[BLACK_KING_SIDE] = 0;
                if(ori == 7) rookCanCastle[BLACK_QUEEN_SIDE] = 0;
            }
        }

        // All other piece when moving to empty square
        board[pos] = board[ori];
        board[ori] = 0;
        
        if(!testingMode){
            enPassantTile = -1;
            enPassantPawnPosition = -1;
            turn = !turn;
        }
        return;
    }  

    // If there is a piece on the target position, capture enemy piece
    capturedPieces.push_back(board[pos]);
    board[pos] = board[ori];
    board[ori] = 0;
    enPassantTile = -1;
    enPassantPawnPosition = -1;
    turn = !turn;
}

bool pieceInPath(int ori, int pos, uint8_t* board) {
    // Board is an 8x8 chess board
    int oriRow = ori / 8;
    int oriCol = ori % 8;
    int posRow = pos / 8;
    int posCol = pos % 8;

    int rowStep = 0;
    int colStep = 0;

    // Determine the direction of movement
    if (oriRow == posRow) {
        // Horizontal movement
        colStep = (posCol > oriCol) ? 1 : -1;
    } else if (oriCol == posCol) {
        // Vertical movement
        rowStep = (posRow > oriRow) ? 1 : -1;
    } else if (abs(oriRow - posRow) == abs(oriCol - posCol)) {
        // Diagonal movement
        rowStep = (posRow > oriRow) ? 1 : -1;
        colStep = (posCol > oriCol) ? 1 : -1;
    } else {
        // Not a valid straight or diagonal move
        return false;
    }

    // Start from the next position in the path
    int currentRow = oriRow + rowStep;
    int currentCol = oriCol + colStep;
    
    // // DEBUG
    // cout << "rs = " << rowStep << " cs = " << colStep << endl;
    // // DEBUG


    // Iterate until we reach the target position
    while (currentRow != posRow || currentCol != posCol) {
        int currentPos = currentRow * 8 + currentCol;
        // cout << " BOARD [" << currentPos << "] = " << board[currentPos] << endl;
        if (board[currentPos] != EMPTY) {
            return true; // Found a piece in the path
        }
        
        currentRow += rowStep;
        currentCol += colStep;
    }

    return false; // No piece found in the path
}

bool canKingCastle(int side, uint8_t * board) {
    int kingPos, rookPos;
    set<int> underAttack;
    if (side == WHITE_KING_SIDE) {
        kingPos = 60; rookPos = 63; 
        if(pieceInPath(kingPos,rookPos,board)) return false;
        underAttack = checkAllPosBeingAttackedBy(BLACK);
        for(int i=kingPos; i<=62; i++){
            if(underAttack.find(i)!=underAttack.end()) return false;
        }
        // // DEBUG
        // cout << "WK HAS NOT MOVED = " << kingHasNotMoved[WHITE_KING] << endl;
        // cout << "ROOK CAN CASTLE = " << rookCanCastle[WHITE_KING_SIDE] << endl;
        // // DEBUG

        if (!kingHasNotMoved[WHITE_KING] || !rookCanCastle[WHITE_KING_SIDE]) return false;
    } else if (side == WHITE_QUEEN_SIDE) {
        kingPos = 60; rookPos = 56; 
        if(pieceInPath(kingPos,rookPos,board)) return false;
        underAttack = checkAllPosBeingAttackedBy(BLACK);
        for(int i=58; i<=kingPos; i++){
            if(underAttack.find(i)!=underAttack.end()) return false;
        }
        if (!kingHasNotMoved[WHITE_KING] || !rookCanCastle[WHITE_QUEEN_SIDE]) return false;
    } else if (side == BLACK_KING_SIDE) {
        kingPos = 4; rookPos = 7; 
        if(pieceInPath(kingPos,rookPos,board)) return false;

        underAttack = checkAllPosBeingAttackedBy(WHITE);
        for(int i=kingPos; i<=6; i++){
            if(underAttack.find(i)!=underAttack.end()) return false;
        }
        if (!kingHasNotMoved[BLACK_KING] || !rookCanCastle[BLACK_KING_SIDE]) return false;
    } else if (side == BLACK_QUEEN_SIDE) {
        kingPos = 4; rookPos = 0; 
        if(pieceInPath(kingPos,rookPos,board)) return false;

        underAttack = checkAllPosBeingAttackedBy(WHITE);
        for(int i=2; i<=kingPos; i++){
            if(underAttack.find(i)!=underAttack.end()) return false;
        }
        if (!kingHasNotMoved[BLACK_KING] || !rookCanCastle[BLACK_QUEEN_SIDE]) return false;
    }

    return true;
}

bool addMove(int r, int c, vector<int> &res, int color, uint8_t * board){
    if (isRowColValid(r, c)) {
        int index = r * 8 + c;
        if (!board[index] || getColor(board[index]) != color) {

            res.push_back(index);
            return board[index] == EMPTY; // Return true if the tile is empty, false if it's occupied
        }
    }
    return 0;
}

bool addMove(int r, int c, set<int> &res, int color, uint8_t * board){
    if (isRowColValid(r, c)) {
        int index = r * 8 + c;
        if (!board[index] || getColor(board[index]) != color) {
            res.insert(index);
            return board[index] == EMPTY;// Return true if the tile is empty, false if it's occupied
        }
    }
    return 0;
}

bool isInCheckAfterMoving(int ori, int pos, uint8_t * board){
    int color = getColor(board[ori]);
    int piece = getPiece(board[ori]);
    uint8_t * tempBoard = new uint8_t[64];
    for(int i=0; i<64; i++){
        tempBoard[i] = board[i];
    }
    
    movePiece(ori, pos, tempBoard, TESTING);

    // // DEBUG 
    // cout << "Color: " << color << endl;
    // cout << "Temp Board: " << endl;
    // displayBoard(tempBoard);
    // bool x = isInCheck(color, tempBoard);
    // cout << "IS IN CHECK???? : "<< x << endl;
    // // DEBUG

    if(isInCheck(color, tempBoard)){
        delete[] tempBoard;
        return true;
    }
    delete[] tempBoard;
    return false;
}

// Need to implement in check
vector<int> checkValidMoves(int pos, uint8_t * board) {
    vector<int> res;

    if (!board[pos]) {
        cout << "Empty tile!" << endl;
        return res;
    }

    int piece = getPiece(board[pos]);
    int color = getColor(board[pos]);
    int directions[8][2] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}, // Vertical and horizontal directions
        {-1, -1}, {1, 1}, {-1, 1}, {1, -1}  // Diagonal directions
    };

    int row = pos / 8;
    int col = pos % 8;

    switch (piece) {
        // Pawn
        case PAWN:
            if (color == WHITE) {
                // Forward moves
                if(!board[(row - 1) * 8 + col]) {
                    addMove(row - 1, col, res, WHITE);
                    if (row == 6 && !board[(row - 2) * 8 + col]) {
                        addMove(row - 2, col, res, WHITE);
                    }
                }
                
                // Diagonal caputures
                if(isRowColValid(row - 1, col - 1)&&board[(row - 1) * 8 + col-1]) addMove(row - 1, col - 1, res, WHITE);
                if(isRowColValid(row - 1, col + 1)&&board[(row - 1) * 8 + col+1]) addMove(row - 1, col + 1, res, WHITE);

                // If enpassant available
                if(isRowColValid(row - 1, col - 1)&&((row - 1) * 8 + col-1)==enPassantTile) addMove(row - 1, col - 1, res, WHITE);
                if(isRowColValid(row - 1, col + 1)&&((row - 1) * 8 + col+1)==enPassantTile) addMove(row - 1, col + 1, res, WHITE);

            } else {
                // Forward moves
                if(!board[(row + 1) * 8 + col]){
                    addMove(row + 1, col, res, BLACK);
                    if (row == 1 && !board[(row + 2) * 8 + col]) {
                        addMove(row + 2, col, res, BLACK);
                    }
                }
                
                // Diagonal caputures
                if(isRowColValid(row + 1, col - 1)&&board[(row + 1) * 8 + col-1]) addMove(row + 1, col - 1, res, BLACK);
                if(isRowColValid(row + 1, col + 1)&&board[(row + 1) * 8 + col+1]) addMove(row + 1, col + 1, res, BLACK);

                // If enpassant available
                if(isRowColValid(row + 1, col - 1)&&((row + 1) * 8 + col-1)==enPassantTile) addMove(row + 1, col - 1, res, BLACK);
                if(isRowColValid(row + 1, col + 1)&&((row + 1) * 8 + col+1)==enPassantTile) addMove(row + 1, col + 1, res, BLACK);
            }
            break;

        // Rook
        case ROOK:
            for (int i = 0; i < 4; ++i) {
                for (int j = 1; j < 8; ++j) {
                    int newRow = row + directions[i][0] * j;
                    int newCol = col + directions[i][1] * j;
                    if (isRowColValid(newRow, newCol)) {
                        int index = newRow * 8 + newCol;
                        if(!addMove(newRow, newCol, res, color, board)) {
                            // cout << "UNSUCCESSFUL ADD, i = "<<i <<" j = " << j << endl;
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
            break;

        // Knight
        case KNIGHT:
            for (int i = -2; i <= 2; ++i) {
                for (int j = -2; j <= 2; ++j) {
                    if (abs(i) != abs(j) && i != 0 && j != 0) {
                        int newRow = row + i;
                        int newCol = col + j;
                        addMove(newRow, newCol, res, color, board);
                    }
                }
            }
            break;

        // Bishop
        case BISHOP:
            for (int i = 4; i < 8; ++i) {
                for (int j = 1; j < 8; ++j) {
                    int newRow = row + directions[i][0] * j;
                    int newCol = col + directions[i][1] * j;
                    if (isRowColValid(newRow, newCol)) {
                        int index = newRow * 8 + newCol;
                        if(!addMove(newRow, newCol, res, color, board)) break;  // LAST TRACKED
                    } else {
                        break;
                    }
                }
            }
            break;

        // Queen
        case QUEEN:
            for (int i = 0; i < 8; ++i) {
                for (int j = 1; j < 8; ++j) {
                    int newRow = row + directions[i][0] * j;
                    int newCol = col + directions[i][1] * j;
                    if (isRowColValid(newRow, newCol)) {
                        int index = newRow * 8 + newCol;
                        if(!addMove(newRow, newCol, res, color, board)) break;
                    } else {
                        break;
                    }
                }
            }
            break;

        // King
        case KING:
            //cout << "DEBUG - " << "IN KING" << " " << endl;

            for (int i = 0; i < 8; ++i) {
                int newRow = row + directions[i][0];
                int newCol = col + directions[i][1];
                addMove(newRow, newCol, res, color, board);
            }

            // // DEBUG
            // cout << "DEBUG - " << "AFTER CHECKING DIRECTIONS" << " " << endl;
            // // DEBUG

            if(turn!=color) return res;

            // Castling
            if(color==WHITE && kingHasNotMoved[WHITE_KING]){
                if(canKingCastle(WHITE_KING_SIDE)) addMove(row, col+2, res, color, board);
                if(canKingCastle(WHITE_QUEEN_SIDE)) addMove(row, col-2, res, color, board);
            } 

            if(color==BLACK && kingHasNotMoved[BLACK_KING]) {
                if(canKingCastle(BLACK_KING_SIDE)) addMove(row, col+2, res, color, board);
                if(canKingCastle(BLACK_QUEEN_SIDE)) addMove(row, col-2, res, color, board);
            }
            
            break;
    }

    // //DEBUG 
    // cout << "VALID MOVES: COLOR = " << color <<" PIECE = " << piece << " at POS = " << pos <<endl;;
    // for(int x:res) cout << x << ' ';
    // cout << endl;
    // //DEBUG 
    
    return res;
}

vector<int> checkAllValidMoves(int color, uint8_t * board){
    vector<int> res;
    for(int i=0; i<64; i++){
        if(board[i] && getColor(board[i]) == color){

            // // DEBUG
            // cout << "\nChecking - pos = " << i << " , piece = " << getPiece(board[i]) << " : ";
            // vector<int> debugVec;
            // // DEBUG

            for(int r: checkValidMoves(i)){
                if(!isInCheckAfterMoving(i, r, board)) {
                    res.push_back(r);
                    // // DEBUG
                    // debugVec.push_back(r);
                    // cout << r << " ";
                    // // DEBUG
                }
            }


            // cout << endl;

            // // DEBUG
            // sort(debugVec.begin(),debugVec.end());
            // cout << "CURR RES MOVES: ";
            // for(int i:debugVec) cout << i << " ";
            // cout << endl;
            // cout << endl;
            // cout << endl;
            // // DEBUG
        }
    }

    // // DEBUG
    // cout << "All VALID MOVES: ";
    // for(int i:res) cout << i << " ";
    // cout << endl;
    // //
    return res;
}

set<int> checkAllPosBeingAttackedBy(int color, uint8_t * board){
    set<int> res;

    // Iterate through all squares on the board
    for (int i = 0; i < 64; ++i) {
        // If the square contains a piece of the specified color
        if (board[i] != EMPTY && getColor(board[i]) == color) {
            int piece = getPiece(board[i]);
            int row = i / 8;
            int col = i % 8;
            switch (piece) {
                // Pawn attacks
                case PAWN:
                    if (color == WHITE) {
                        if (isRowColValid(row - 1, col - 1)) addMove(row - 1, col - 1, res, color);
                        if (isRowColValid(row - 1, col + 1)) addMove(row - 1, col + 1, res, color);
                    } else {
                        if (isRowColValid(row + 1, col - 1)) addMove(row + 1, col - 1, res, color);
                        if (isRowColValid(row + 1, col + 1)) addMove(row + 1, col + 1, res, color);
                    }
                    break;
                // Other pieces use checkValidMoves
                default:
                    // // Debug
                    // cout << "DEBUG - starting with " << i << endl;
                    // // Debug

                    vector<int> moves = checkValidMoves(i, board);
                    for(int move:moves) res.insert(move);

                    // // Debug
                    // cout << "DEBUG - done with " << i << ", moves: ";
                    // for(int move:moves) cout << move << " ";
                    // cout << endl;
                    // // Debug

                    break;
            }
        }
    }
    return res;
}

vector<int> filterValidMoves(int ori, vector<int> valids, uint8_t * board){
    vector<int> filtered;
    for(int i:valids){
        if(!isInCheckAfterMoving(ori,i)) filtered.push_back(i);
    }
    return filtered;
}

bool isInCheck(int color, uint8_t * board){
    int enemyColor = WHITE;
    if(color == WHITE) enemyColor = BLACK;

    // //DEBUG
    // cout << "IN isInCheck " << endl;
    // displayBoard(board);
    // cout << "isInCheck BOARD ABOVE" << endl;
    // //DEBUG

    set<int> enemyAttacks = checkAllPosBeingAttackedBy(enemyColor, board);

    for(int i:enemyAttacks){
        if(getColor(board[i])==color && getPiece(board[i])==KING){
            return true;
        }
    }

    return false;
}

bool isCheckMate(int color, uint8_t * board){
    int kingpos = -1;
    for(int i=0; i<64; i++){
        if(board[i] == (color | KING)){
            kingpos = i; break;
        }
    }
    return isInCheck(color) && checkAllPosBeingAttackedBy(color).empty();
}

// // PERFORMANCE TESTING
// // Need to implement
// unsigned long long perft(int depth, uint8_t * board){
//     unsigned long long res = 0;
//     uint8_t * tempBoard = new uint8_t[64];
//     if(depth == 0) return res;

//     delete[] tempBoard;
//     return perft(depth-1, tempBoard);
// }


// Driver
int main(){
    // const char* fen = "rnbqkbnr/pp1ppppp/2p5/8/8/8/PPPPPPPP/RNBQKBNR"; // "w KQkq - 0 1"
    // const char* fen = "r3k2r/p4P1p/2nqbnpb/1ppp1P2/PPPP4/8/5KPP/RNBQ1BNR";

    // Performance test: https://www.chessprogramming.org/Perft_Results
    
    // const char* fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R"; // " w KQkq - ";
    // const char* fen = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ";
    // const char* fen = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1"; // " w kq - 0 1";
    // const char* fen = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R"; // " w KQ - 1 8 ";
    // const char* fen = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1"; // "w - - 0 10"
    // const char* fen = "R3k2r/8/3p4/2P5/8/8/P7/8";

    const char* fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR";
    enPassantPawnPosition = -1;
    enPassantTile = -1;
    kingHasNotMoved[WHITE_KING] = 1;
    kingHasNotMoved[BLACK_KING] = 1;
    rookCanCastle[WHITE_KING_SIDE] = 1;
    rookCanCastle[WHITE_QUEEN_SIDE] = 1;
    rookCanCastle[BLACK_KING_SIDE] = 1;
    rookCanCastle[BLACK_QUEEN_SIDE] = 1;
    turn = 0;
    
    initBoard(fen);

    cout<<endl;

    displayBoard();

    // set<int> beingAttacked = checkAllPosBeingAttackedBy(BLACK);

    // cout << "Pos being attacked by BLACK: ";
    // for(int i:beingAttacked){
    //     cout << i << ' ';
    // }
    // cout << endl;

    // set<int> beingAttackedEnemy = checkAllPosBeingAttackedBy(WHITE);
    // cout << "Pos being attacked by WHITE: ";
    // for(int i:beingAttackedEnemy){
    //     cout << i << ' ';
    // }
    // cout << endl;

    // cout << "WHITE in check = " << isInCheck(WHITE) << endl;

    cout << endl;
    vector<int> allVals = checkAllValidMoves(WHITE);
    sort(allVals.begin(),allVals.end());
    cout << "WHITE valids: ";
    for(int i:allVals){
        cout << i << " ";
    }
    cout << endl;

    cout << "WHITE valids cout: " << allVals.size() << endl;

}


