#include<iostream>
#include<limits>
#include <chrono>
#include <ctime>
#include <random>
#include <algorithm>
#include "engine.hpp"
#include <unordered_map>
#include "board.hpp"

constexpr U8 cw_90[64] = {
    48, 40, 32, 24, 16, 8,  0,  7,
    49, 41, 33, 25, 17, 9,  1,  15,
    50, 42, 18, 19, 20, 10, 2,  23,
    51, 43, 26, 27, 28, 11, 3,  31,
    52, 44, 34, 35, 36, 12, 4,  39,
    53, 45, 37, 29, 21, 13, 5,  47,
    54, 46, 38, 30, 22, 14, 6,  55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 acw_90[64] = {
     6, 14, 22, 30, 38, 46, 54, 7,
     5, 13, 21, 29, 37, 45, 53, 15,
     4, 12, 18, 19, 20, 44, 52, 23,
     3, 11, 26, 27, 28, 43, 51, 31,
     2, 10, 34, 35, 36, 42, 50, 39,
     1,  9, 17, 25, 33, 41, 49, 47,
     0,  8, 16, 24, 32, 40, 48, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 cw_180[64] = {
    54, 53, 52, 51, 50, 49, 48, 7,
    46, 45, 44, 43, 42, 41, 40, 15,
    38, 37, 18, 19, 20, 33, 32, 23,
    30, 29, 26, 27, 28, 25, 24, 31,
    22, 21, 34, 35, 36, 17, 16, 39,
    14, 13, 12, 11, 10,  9,  8, 47,
     6,  5,  4,  3,  2,  1,  0, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

constexpr U8 id[64] = {
     0,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 63
};

#define cw_90_pos(p) cw_90[p]
#define cw_180_pos(p) cw_180[p]
#define acw_90_pos(p) acw_90[p]
#define cw_90_move(m) move_promo(cw_90[getp0(m)], cw_90[getp1(m)], getpromo(m))
#define acw_90_move(m) move_promo(acw_90[getp0(m)], acw_90[getp1(m)], getpromo(m))
#define cw_180_move(p) move_promo(cw_180[getp0(m)], cw_180[getp1(m)], getpromo(m))
#define color(p) ((PlayerColor)(p & (WHITE | BLACK)))



using namespace std;
class TimeException : public exception{
    public:
        const char * what() const throw(){
            return "Threshold Time exceeded\n";
        }
};
bool current_player = false;
vector<vector<int>> create_value_matrix(const Board& b);

// -------------------------------------------------------CREATING ROLLERBOARD FROM BOARD----------------------------------------------------------------
class RollerBoard{
    public:
        Board b;
        vector<U16> moves;
        vector<RollerBoard*> children;
        int eval_val =0;
        vector<vector<int>> value_matrix;
        unordered_set<U16> future_moves;
        RollerBoard(const Board& b);
        void create_states();
        bool _under_threat(U8 piece_pos) const;
        unordered_set<U16> _get_pseudolegal_moves_for_side(U8 color) const;
        unordered_set<U16> _get_pseudolegal_moves_for_piece(U8 piece_pos) const;
};

RollerBoard::RollerBoard(const Board& b){
    this->b = b;
    this->value_matrix = create_value_matrix(b);
    this->future_moves = b.get_legal_moves();
}

void RollerBoard::create_states(){
    // cout<<"in create states"<<endl;
    unordered_set<U16> future_moves = this->future_moves;
    // cout<<"size is "<<future_moves.size();
    for(auto i:future_moves){
        Board* temp = this->b.copy();
        temp->do_move(i);
        RollerBoard* rtemp = new RollerBoard(*temp);
        rtemp->moves.push_back(i);
        this->children.push_back(rtemp);
    }
}

int min_val( const RollerBoard& board, double alpha, double beta, int d, chrono::time_point<chrono::system_clock>start, double time_limit, double parent_val);
int max_val( const RollerBoard& board, double alpha, double beta, int d, chrono::time_point<chrono::system_clock>start, double time_limit, double parent_val);
int max_quisecent(const RollerBoard& board , int depth , double parent_val);
int min_quisecent(const RollerBoard& board , int depth , double parent_val);

int get_y(U8 p){
    return ((p)>>3);
}

int get_x(U8 p){
    return ((p)&0x7);
}

U8 get_p1(U16 m){
    return ((m)&0x3f);
}

U8 get_p0(U16 m){    
    return (((m)>>8)&0x3f);
}

U8 get_pos(int x,int y){
    return (((y)<<3)|(x));
}

vector<vector<int>> create_value_matrix(const Board& p){
    vector<vector<int>> val(8, vector<int>(8,0));
    U8 b_rook_ws = p.data.b_rook_ws;
    U8 b_rook_bs = p.data.b_rook_bs;
    U8 b_pawn_ws = p.data.b_pawn_ws;
    U8 b_pawn_bs = p.data.b_pawn_bs;
    U8 b_bishop = p.data.b_bishop;
    U8 w_rook_ws = p.data.w_rook_ws;
    U8 w_rook_bs = p.data.w_rook_bs;
    U8 w_pawn_ws = p.data.w_pawn_ws;
    U8 w_pawn_bs = p.data.w_pawn_bs;
    U8 w_bishop = p.data.w_bishop;
    val[get_x(b_rook_ws)][get_y(b_rook_ws)] = -5;
    val[get_x(b_rook_bs)][get_y(b_rook_bs)] = -5;
    val[get_x(b_pawn_ws)][get_y(b_pawn_ws)] = -1;
    val[get_x(b_pawn_bs)][get_y(b_pawn_bs)] = -1;
    val[get_x(b_bishop)][get_y(b_bishop)] = -4;
    val[get_x(w_rook_ws)][get_y(w_rook_ws)] = 5;
    val[get_x(w_rook_bs)][get_y(w_rook_bs)] = 5;
    val[get_x(w_pawn_ws)][get_y(w_pawn_ws)] = 1;
    val[get_x(w_pawn_bs)][get_y(w_pawn_bs)] = 1;
    val[get_x(w_bishop)][get_y(w_bishop)] = 4;
    return val;
}

unordered_set<U16> my_transform_moves(const unordered_set<U16>& moves, const U8 *transform) {

    unordered_set<U16> rot_moves;

    for (U16 move : moves) {
        rot_moves.insert(move_promo(transform[getp0(move)], transform[getp1(move)], getpromo(move)));
    }

    return rot_moves;
}

unordered_set<U16> my_construct_bottom_rook_moves_with_board(const U8 p0, const U8* board) {

    int left_rook_reflect[7] = {0, 8, 16, 24, 32, 40, 48};
    PlayerColor color = color(board[p0]);
    unordered_set<U16> rook_moves;
    bool refl_blocked = false;

    if (p0 < 8 || p0 == 13) {
        if (!(board[p0+pos(0,1)] & color)) rook_moves.insert(move(p0, p0+pos(0,1))); // top
        if (p0 == 1) { // top continued on the edge
            for (int y = 1; y<=6; y++) {
                U8 p1 = pos(1, y);
                if (board[p1]) {
                    if (board[p1] & color) break;         // our piece
                    else rook_moves.insert(move(p0, p1)); // their piece - capture
                    break;
                }
                else rook_moves.insert(move(p0, p1));
            }
        }
    }
    if (p0 >= 8) {
        if (!(board[p0-pos(0,1)] & color)) rook_moves.insert(move(p0, p0-pos(0,1))); // bottom
    }

    if (p0 != 6) {
        if (!(board[p0+pos(1,0)] & color)) rook_moves.insert(move(p0, p0+pos(1,0))); // right
    }

    for (int x=getx(p0)-1; x>=0; x--) {
        U8 p1 = pos(x, gety(p0));
        if (board[p1]) {
            refl_blocked = true;
            if (board[p1] & color) break;         // our piece
            else rook_moves.insert(move(p0, p1)); // their piece - capture
            break;
        }
        else {
            rook_moves.insert(move(p0, p1));
        }
    }

    if (refl_blocked) return rook_moves;
    
    if (p0 < 8) {
        for (int p1 : left_rook_reflect) {
            if (board[p1]) {
                if (board[p1] & color) break;         // our piece
                else rook_moves.insert(move(p0, p1)); // their piece - capture
                break;
            }
            else {
                rook_moves.insert(move(p0, p1));
            }
        }
    }

    return rook_moves;
}

unordered_set<U16> my_construct_bottom_bishop_moves_with_board(const U8 p0, const U8* board) {

    PlayerColor color = color(board[p0]);
    unordered_set<U16> bishop_moves;

    // top right - move back
    if (p0 < 6 || p0 == 13) {
        if (!(board[p0+pos(0,1)+pos(1,0)] & color)) bishop_moves.insert(move(p0, p0+pos(0,1)+pos(1,0)));
    }
    // bottom right - move back
    if (p0 > 6) {
        if (!(board[p0-pos(0,1)+pos(1,0)] & color)) bishop_moves.insert(move(p0, p0-pos(0,1)+pos(1,0)));
    }

    vector<U8> p1s;
    vector<U8> p1s_2;

    // top left - forward / reflections
    if (p0 == 1) {
        p1s.push_back(pos(0,1));
        p1s.push_back(pos(1,2));
    }
    else if (p0 == 2) {
        p1s.push_back(pos(1,1));
        p1s.push_back(pos(0,2));
        p1s.push_back(pos(1,3));
    }
    else if (p0 == 3) {
        p1s.push_back(pos(2,1));
        p1s.push_back(pos(1,2));
        p1s.push_back(pos(0,3));
        p1s.push_back(pos(1,4));
        p1s.push_back(pos(2,5));
        p1s.push_back(pos(3,6));
    }
    else if (p0 == 4 || p0 == 5) {
        p1s.push_back(p0+pos(0,1)-pos(1,0));
        p1s.push_back(p0-pos(2,0));
    }
    else if (p0 == 6) {
        p1s.push_back(pos(5,1));
    }
    else if (p0 == 10) {
        p1s_2.push_back(pos(1,0));
        p1s_2.push_back(pos(0,1));

        p1s.push_back(pos(1,2));
        p1s.push_back(pos(0,3));
        p1s.push_back(pos(1,4));
        p1s.push_back(pos(2,5));
        p1s.push_back(pos(3,6));
    }
    else if (p0 == 11) {
        p1s.push_back(pos(2,0));
        p1s.push_back(pos(1,1));
        p1s.push_back(pos(0,2));
    }
    else if (p0 == 12) {
        p1s.push_back(pos(3,0));
        p1s.push_back(pos(2,1));
        p1s.push_back(pos(1,2));
        p1s.push_back(pos(0,3));
    }
    else if (p0 == 13) {
        p1s.push_back(pos(4,0));
        p1s.push_back(pos(3,1));
    }

    for (auto p1 : p1s) {
        if (board[p1]) {
            if (board[p1] & color) break;           // our piece
            else bishop_moves.insert(move(p0, p1)); // their piece - capture
            break;
        }
        else {
            bishop_moves.insert(move(p0, p1));
        }
    }

    for (auto p1 : p1s_2) {
        if (board[p1]) {
            if (board[p1] & color) break;           // our piece
            else bishop_moves.insert(move(p0, p1)); // their piece - capture
            break;
        }
        else {
            bishop_moves.insert(move(p0, p1));
        }
    }

    return bishop_moves;
}

unordered_set<U16> my_construct_bottom_pawn_moves_with_board(const U8 p0, const U8 *board, bool promote = false) {
    
    PlayerColor color = color(board[p0]);
    unordered_set<U16> pawn_moves;

    if (!(board[pos(getx(p0)-1,0)] & color)) {
        if (promote) {
            pawn_moves.insert(move_promo(p0, pos(getx(p0)-1,0), PAWN_ROOK));
            pawn_moves.insert(move_promo(p0, pos(getx(p0)-1,0), PAWN_BISHOP));
        }
        else {
            pawn_moves.insert(move(p0, pos(getx(p0)-1,0)));
        }
    }
    if (!(board[pos(getx(p0)-1,1)] & color)) {
        if (promote) {
            pawn_moves.insert(move_promo(p0, pos(getx(p0)-1,1), PAWN_ROOK));
            pawn_moves.insert(move_promo(p0, pos(getx(p0)-1,1), PAWN_BISHOP));
        }
        else {
            pawn_moves.insert(move(p0, pos(getx(p0)-1,1)));
        }
    }
    if (p0 == 10 && !(board[17] & color)) pawn_moves.insert(move(p0, 17));

    return pawn_moves;
}

unordered_set<U16> my_construct_bottom_king_moves_with_board(const U8 p0, const U8 *board) {

    // king can't move into check. See if these squares are under threat from 
    // enemy pieces as well.
    
    PlayerColor color = color(board[p0]);
    unordered_set<U16> king_moves;
    if (!(board[pos(getx(p0)-1,0)] & color)) king_moves.insert(move(p0, pos(getx(p0)-1,0)));
    if (!(board[pos(getx(p0)-1,1)] & color)) king_moves.insert(move(p0, pos(getx(p0)-1,1)));
    if (p0 == 10 && !(board[pos(getx(p0)-1,2)] & color)) king_moves.insert(move(p0, pos(getx(p0)-1,2)));
    if (p0 != 6 && !(board[pos(getx(p0)+1,0)] & color)) king_moves.insert(move(p0, pos(getx(p0)+1,0)));
    if (p0 != 6 && !(board[pos(getx(p0)+1,1)] & color)) king_moves.insert(move(p0, pos(getx(p0)+1,1)));
    if (p0 >= 12 && !(board[pos(getx(p0)+1,2)] & color)) king_moves.insert(move(p0, pos(getx(p0)+1,2)));
    if (p0 == 13 && !(board[pos(getx(p0),2)] & color)) king_moves.insert(move(p0, pos(getx(p0),2)));
    if (!(board[pos(getx(p0),gety(p0)^1)] & color)) king_moves.insert(move(p0, pos(getx(p0),gety(p0)^1)));

    return king_moves;
}

unordered_set<U16> RollerBoard::_get_pseudolegal_moves_for_piece(U8 piece_pos) const {

    unordered_set<U16> moves;
    U8 piece_id = this->b.data.board_0[piece_pos];

    unordered_set<U8> bottom({ 1, 2, 3, 4, 5, 6, 10, 11, 12, 13 });
    unordered_set<U8> left({ 0, 8, 16, 24, 32, 40, 9, 17, 25, 33 });
    unordered_set<U8> top({ 48, 49, 50, 51, 52, 53, 41, 42, 43, 44 });
    unordered_set<U8> right({ 54, 46, 38, 30, 22, 14, 45, 37, 29, 21 });

    const U8 *board = this->b.data.board_0;
    const U8 *coord_map = id;
    const U8 *inv_coord_map = id;
    if      (left.count(piece_pos))  { board = this->b.data.board_270;  coord_map = acw_90; inv_coord_map = cw_90;  }
    else if (top.count(piece_pos))   { board = this->b.data.board_180; coord_map = cw_180; inv_coord_map = cw_180; }
    else if (right.count(piece_pos)) { board = this->b.data.board_90; coord_map = cw_90;  inv_coord_map = acw_90; }

    if (piece_id & PAWN) {
        if (((piece_pos == 51 || piece_pos == 43) && (piece_id & WHITE)) || 
            ((piece_pos == 11 || piece_pos == 3)  && (piece_id & BLACK)) ) {
            moves = my_construct_bottom_pawn_moves_with_board(coord_map[piece_pos], board, true);
        }
        else {
            moves = my_construct_bottom_pawn_moves_with_board(coord_map[piece_pos], board);
        }
    }
    else if (piece_id & ROOK) {
        moves = my_construct_bottom_rook_moves_with_board(coord_map[piece_pos], board);
    }
    else if (piece_id & BISHOP) {
        moves = my_construct_bottom_bishop_moves_with_board(coord_map[piece_pos], board);
    }
    else if (piece_id & KING) {
        moves = my_construct_bottom_king_moves_with_board(coord_map[piece_pos], board);
    }

    moves = my_transform_moves(moves, inv_coord_map);

    return moves;
}

unordered_set<U16> RollerBoard::_get_pseudolegal_moves_for_side(U8 color) const {
    unordered_set<U16> pseudolegal_moves;
    U8 *pieces = (U8*)(&(this->b.data));
    if (color == WHITE) {
        pieces = pieces + 6;
    }
    for (int i=0; i<6; i++) {
        if (pieces[i] == DEAD) continue;
        auto piece_moves = this->_get_pseudolegal_moves_for_piece(pieces[i]);
        pseudolegal_moves.insert(piece_moves.begin(), piece_moves.end());
    }
    return pseudolegal_moves;
}

bool RollerBoard::_under_threat(U8 piece_pos) const {

    auto pseudolegal_moves = this->_get_pseudolegal_moves_for_side(this->b.data.player_to_play ^ (WHITE | BLACK));

    for (auto move : pseudolegal_moves) {
        if (getp1(move) == piece_pos) {
            return true;
        }
    }
    return false;
}

// --------------------------------------------------------TRANSPOSITION TABLE--------------------------------------------------------------------
unsigned long long int random_table[8][8][6];
mt19937 mt(01234567);

unsigned long long int randomInt(){
    uniform_int_distribution<unsigned long long int> dist(0,UINT64_MAX);
    return dist(mt);
}

void initTable(){
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            for(int k=0;k<6;k++){
                random_table[i][j][k] = randomInt();
            }
        }
    }
}

int get_index(int piece_value){
    if(piece_value == -1) return 0;
    if(piece_value == 1) return 1;
    if(piece_value == -4) return 2;
    if(piece_value == 4) return 3;
    if(piece_value == -5) return 4;
    if(piece_value == 5) return 5;
    else return -1;
}

unsigned long long int get_hash(vector<vector<int>> val){
    unsigned long long int h = 0;
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){
            if(val[i][j]!=0){
                int piece = get_index(val[i][j]);
                h ^= random_table[i][j][piece];
            }
        }
    }
    return h;
}
unordered_map<unsigned long long int, double> transposition_table;

// ---------------------------------------------------------------EVALUATION FUNCTION-----------------------------------------------------------------

double compute_piece_value(RollerBoard* rb){
    double white = 0;
    double black = 0;
    if(static_cast<int>(rb->b.data.b_king) != 63 ) black += 6;
    if(static_cast<int>(rb->b.data.w_king) != 63 ) white += 6;
    if(static_cast<int>(rb->b.data.b_rook_ws) != 63 ) black += 4.5;
    if(static_cast<int>(rb->b.data.b_rook_bs) != 63 ) black += 4.5;
    if(static_cast<int>(rb->b.data.b_pawn_ws) != 63 ) black += 1.5;
    if(static_cast<int>(rb->b.data.b_pawn_bs) != 63 ) black += 1.5;
    if(static_cast<int>(rb->b.data.b_bishop) != 63 ) black += 3.5;
    if(static_cast<int>(rb->b.data.w_rook_ws) != 63 ) white += 4.5;
    if(static_cast<int>(rb->b.data.w_rook_bs) != 63 ) white += 4.5;
    if(static_cast<int>(rb->b.data.w_pawn_ws) != 63 ) white += 1.5;
    if(static_cast<int>(rb->b.data.w_pawn_bs) != 63 ) white += 1.5;
    if(static_cast<int>(rb->b.data.w_bishop) != 63 ) white += 3.5;
    // cout<<"POSITION OF ROOK=========================="<<static_cast<int>(rb->b.data.board_0[16])<<endl;
    // for(int i=0;i<63;i++){
    //         if(rb->b.data.board_0[i]== BLACK|ROOK){ black += 4.5; }
    //         else if(rb->b.data.board_0[i]== BLACK|PAWN){ black += 1.5; }
    //         else if(rb->b.data.board_0[i]== BLACK|BISHOP){ black += 3.5; }
    //         else if(rb->b.data.board_0[i]== BLACK|KING){ black += 6; }
    //         else if(rb->b.data.board_0[i]== WHITE|PAWN){ white += 1.5; }
    //         else if(rb->b.data.board_0[i]== WHITE|KING){ white += 6; }
    //         else if(rb->b.data.board_0[i]== WHITE|BISHOP){ white += 3.5; }
    //         else if(rb->b.data.board_0[i]== WHITE|ROOK){ white += 4.5; }
    // }
    // if(rb->b.data.player_to_play == WHITE){
    //     cout<<"white-black*************************************** "<<white-black<<" white is----------------"<<white<<" black is-------------"<<black<<endl;
    //     return white-black;
    // }
    // else{
    //     return black-white;
    // }
    if(current_player == true){ 
        // cout<<"white-black*************************************** "<<white-black<<" white is----------------"<<white<<" black is-------------"<<black<<endl;
        return white-black;}
    return black-white;
}

vector<int> get_my_pieces(RollerBoard* rb){
    vector<int> my_pieces;
    int pawn_val = 0;
    int bishop_val = 0;
    int rook_val = 0;
    // int king_val = 0;
    for(int i=0;i<7;i++){
        for(int j=0;j<7;j++){
            if(rb->value_matrix[i][j]==1) pawn_val+=1;
            else if(rb->value_matrix[i][j]==5) rook_val+=1;
            else if(rb->value_matrix[i][j]==4) bishop_val+=1;
        }
    }
    my_pieces = {1,bishop_val,pawn_val,rook_val};
    return my_pieces;
}

vector<int> get_opp_pieces(RollerBoard* rb){
    vector<int> opp_pieces;
    int pawn_val = 0;
    int bishop_val = 0;
    int rook_val = 0;
    // int king_val = 0;
    for(int i=0;i<7;i++){
        for(int j=0;j<7;j++){
            if(rb->value_matrix[i][j]==-1) pawn_val+=1;
            else if(rb->value_matrix[i][j]==-5) rook_val+=1;
            else if(rb->value_matrix[i][j]==-4) bishop_val+=1;
        }
    }
    opp_pieces = {1,bishop_val,pawn_val,rook_val};
    return opp_pieces;
}


bool isPathClear(U8 start, U8 end, RollerBoard* rb){
    if(get_y(start)==get_y(end)){
        int dir=(get_x(start)<get_x(end))? 1: -1;
        for(int x = get_x(start)+dir; x!=get_x(end);x+=dir){
            if(rb->b.data.board_0[get_pos(x,get_y(start))]!=0){
                return false;
            }
        }
    }
    else if(get_x(start)==get_x(end)){
        int dir = (get_y(start)<get_y(end))?1:-1;
        for(int y=get_y(start)+dir;y!=get_y(end);y+=dir){
            if(rb->b.data.board_0[get_pos(get_x(start),y)]!=0){
                return false;
            }
        }
    }
    return true;
}


double evaluate_pawn_promotion(RollerBoard* rb){
    vector<U8> white_pos = {get_pos(2,6), get_pos(3,6), get_pos(2,5)};
    vector<U8> black_pos = {get_pos(4,1), get_pos(4,0), get_pos(3,0)};
    double black_score =0;
    double white_score =0;
    U8 w_p1 = rb->b.data.w_pawn_bs;
    U8 w_p2 = rb->b.data.w_pawn_ws;
    U8 b_p1 = rb->b.data.b_pawn_bs;
    U8 b_p2 = rb->b.data.b_pawn_ws;
    //----------------------w_p1--------------------------------
    for(U8 e:white_pos){
        if(isPathClear(w_p1,e,rb)){
            double distance = abs(get_x(e)-get_x(w_p1))+abs(get_y(e)-get_y(w_p1));
            white_score += (14-distance)*0.3;
        }
        if(isPathClear(w_p2,e,rb)){
            double distance = abs(get_x(e)-get_x(w_p2))+abs(get_y(e)-get_y(w_p2));
            white_score += (14-distance)*0.3;
        }
    }
    for(U8 e:black_pos){
        if(isPathClear(b_p1,e,rb)){
            double distance = abs(get_x(e)-get_x(b_p1))+abs(get_y(e)-get_y(b_p1));
            black_score += (14-distance)*0.3;
        }
        if(isPathClear(b_p2,e,rb)){
            double distance = abs(get_x(e)-get_x(b_p2))+abs(get_y(e)-get_y(b_p2));
            black_score += (14-distance)*0.3;
        }
    } 
    if(current_player==true){ 
        // cout<<"Pawn value is++++++++++++++++++++++++++++++++        "<<black_score<<"       "<<white_score<<endl;
        return white_score-black_score;}
    else{ return black_score-white_score;}

}
int compute_pawn_promotion(RollerBoard* rb){
    int w_score = 0;
    int b_score = 0;
    U8 w_p_1 = rb->b.data.w_pawn_bs;
    if(get_x(w_p_1)>=2 && get_x(w_p_1)<5 && get_y(w_p_1) == 6){ w_score+= 30;}
    U8 w_p_2 = rb->b.data.w_pawn_ws;
    if(get_x(w_p_2)>=2 && get_x(w_p_2)<5 && get_y(w_p_2) == 6){ w_score+= 30;}
    U8 b_p_1 = rb->b.data.b_pawn_bs;
    if(get_x(b_p_1)>=2 && get_x(b_p_1)<5 && get_y(b_p_1) == 0){ b_score+= 30;}
    U8 b_p_2 = rb->b.data.b_pawn_ws;
    if(get_x(b_p_2)>=2 && get_x(b_p_2)<5 && get_y(b_p_2) == 0){ b_score+= 30;}
    if(current_player == true){ return w_score-b_score;}
    return b_score-w_score;
}

double king_safety(RollerBoard* rb){
    int adjacent[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    double black_threat_count = 0;
    double white_threat_count = 0;
    bool flag = false;
    if(rb->b.data.player_to_play == WHITE) flag = true;
    U8 king_w = rb->b.data.w_king;
    int king_w_x = get_x(king_w);
    int king_w_y = get_y(king_w);
    U8 king_b = rb->b.data.b_king;
    int king_b_x = get_x(king_b);
    int king_b_y = get_y(king_b);
    for(int i=0;i<8;i++){
        int new_w_X = king_w_x+adjacent[i][0]; 
        int new_w_Y = king_w_x+adjacent[i][1];
        int new_b_X = king_b_x+adjacent[i][0];
        int new_b_Y = king_b_y+adjacent[i][1];
        // if(flag == true){
        if(new_w_X>=0 && new_w_X<7 && new_w_Y>=0 && new_w_X<7){
            U8 p =rb->b.data.board_0[get_pos(new_w_X,new_w_Y)];
            if( p == BLACK|ROOK) white_threat_count+= 4.5;
            else if( p== BLACK|BISHOP) white_threat_count+= 3.5;
            else if( p==BLACK|PAWN) white_threat_count+= 1.5;
            else if( p==BLACK|KING ) white_threat_count+= 6;
        }
        // }
        // else{
        if(new_b_X>=0 && new_b_X<7 && new_b_Y>=0 && new_b_X<7){
            U8 p =rb->b.data.board_0[get_pos(new_b_X,new_b_Y)];
            if( p == WHITE|ROOK) black_threat_count+= 4.5;
            else if( p== WHITE|BISHOP) black_threat_count+= 3.5;
            else if( p==WHITE|PAWN) black_threat_count+= 1.5;
            else if( p==WHITE|KING ) black_threat_count+= 6;
        }  
        // }      
    }
    if(current_player==true){
        // cout<<"black_king --------"<<black_threat_count<<" white_king -------- "<<white_threat_count<<endl;
        return black_threat_count-white_threat_count;}
    return white_threat_count-black_threat_count;
}


int compute_check_moves(RollerBoard* rb){
    bool flag = false;
    if(rb->b.data.player_to_play == WHITE) flag = true;
    bool check = false;
    if(rb->b.in_check()){ check = true;}
    if(flag){
        if(check) return -1000;
        else return 10;
    }
    else{
        if(check) return 1000;
        else return -10;
    }
}

double compute_threats(RollerBoard* rb){
    U8 b_others[6] = {rb->b.data.b_rook_bs,rb->b.data.b_rook_ws,rb->b.data.b_king,rb->b.data.b_pawn_bs,rb->b.data.b_pawn_ws,rb->b.data.b_bishop};
    U8 w_others[6] = {rb->b.data.w_rook_bs,rb->b.data.w_rook_ws,rb->b.data.w_king,rb->b.data.w_pawn_bs,rb->b.data.w_pawn_ws,rb->b.data.w_bishop};
    double white_threats = 0;
    double black_threats = 0;
    if(rb->_under_threat(b_others[0])){ black_threats+= 9;}
    if(rb->_under_threat(b_others[1])){ black_threats+= 9;}
    if(rb->_under_threat(b_others[2])){ black_threats+= 12;}
    if(rb->_under_threat(b_others[3])){ black_threats+= 3;}
    if(rb->_under_threat(b_others[4])){ black_threats+= 3;}
    if(rb->_under_threat(b_others[5])){ black_threats+= 7;}

    if(rb->_under_threat(w_others[0])){ white_threats+= 9;}
    if(rb->_under_threat(w_others[1])){ white_threats+= 9;}
    if(rb->_under_threat(w_others[2])){ white_threats+= 12;}
    if(rb->_under_threat(w_others[3])){ white_threats+= 3;}
    if(rb->_under_threat(w_others[4])){ white_threats+= 3;}
    if(rb->_under_threat(w_others[5])){ white_threats+= 7;}    
    // cout<<"----------------------the threats difference---------------------- "<<"black_threats "<<black_threats<<" white threats "<<white_threats<<endl;
    // cout<<"CURRENT PLAYER "<<current_player<<endl;
    if(current_player == true){ 
        return black_threats-white_threats;
    }
    return white_threats-black_threats;
}

int compute_possible_moves(RollerBoard* rb){
    return rb->future_moves.size();
}

double normalize(double value, double min, double max){
    return (value-min)/(max-min);
}

double evaluation(RollerBoard* b){
    // if(b->b.data.player_to_play == WHITE) cout<<"WHITE"<<endl;
    // else cout<<"----------------BLACK----------------"<<endl;
    // double c = normalize(compute_piece_value(b),-655,655);
    // double k = normalize(compute_pawn_promotion(b),-60,60);
    // double m = normalize(compute_check_moves(b),-1000,1000);
    // double n = normalize(compute_threats(b),-25.5,25.5);
    // double a = normalize(king_safety(b),-95,95);
    double c = compute_piece_value(b);
    double n = compute_threats(b);
    double a = king_safety(b);
    // double p = evaluate_pawn_promotion(b);
    return  c*1.5 +n*1.5 + a*2;
}

// --------------------------------------------------------------------SEARCH--------------------------------------------------------------------

// ------------------------------------------------------Search------------------------------------------------------------------------------------
bool max_compare(RollerBoard* r1, RollerBoard* r2){
    return r1->eval_val>r2->eval_val;
}
bool min_compare(RollerBoard* r1, RollerBoard* r2){
    return r1->eval_val<r2->eval_val;
}
double max_quisecent(RollerBoard* b, int depth, double parent_val);
double min_quisecent(RollerBoard* b, int depth, double parent_val){
    // cout<<"min_q begin "<<endl;
    double e = evaluation(b);

    // cout<<"min_q after eval "<<e<<" parent_val "<<parent_val<<" depth is "<<depth<<endl;
    if((e-parent_val>100 || e-parent_val<100) && depth!=0){
        b->create_states();
        double v = numeric_limits<double>::max();  // maximum value
        // cout<<"min_q before_loop"<<endl;
        for(int i=0;i<b->children.size();i++){
            // cout<<"min_q in loop before max_q"<<endl;
            v = min(v,max_quisecent(b->children[i],depth-1,e));
            // cout<<"min_q in loop after max_q"<<endl;
        }
        // cout<<"output of min_q is "<<v<<endl;
        return v;
    }
    else{
        // cout<<"min_q in else "<<e<<endl;
        return e;
    }
}

double max_quisecent(RollerBoard* b, int depth, double parent_val){
    // cout<<"max_q begin "<<endl;
    double e = evaluation(b);
    // cout<<"max_q after eval "<<e<<" parent_val "<<parent_val<<" depth is "<<depth<<endl;
    if((e-parent_val>100 || e-parent_val<100) && depth!=0){
        b->create_states();
        double v = numeric_limits<double>::min();  // minimum value
        // cout<<"max_q before_loop"<<endl;
        for(int i=0;i<b->children.size();i++){
            // cout<<"max_q in loop before max_q"<<endl;
            v = max(v,min_quisecent(b->children[i],depth-1,e));
            // cout<<"max_q in loop after min_q"<<endl;
        }
        // cout<<"output of max_q is "<<v<<endl;
        return v;
    }
    else{
        // cout<<"max_q in else "<<e<<endl;
        return e;
    }
}
double min_val(RollerBoard* rb, double alpha, double beta, int d, chrono::time_point<chrono::system_clock>start, double time_limit, double parent_val);
double max_val(RollerBoard* rb, double alpha, double beta, int d, chrono::time_point<chrono::system_clock>start, double time_limit, double parent_val){
    chrono::duration<double> elapsed_time = chrono::system_clock::now()-start;
    // cout<<"Begin\n";
    if (elapsed_time.count()>time_limit){
        TimeException t;
        throw t;
    }
    vector<int> my_pieces = get_my_pieces(rb);
    vector<int> opp_pieces = get_opp_pieces(rb);
    int my_total = my_pieces[0]+my_pieces[1]+my_pieces[2]+my_pieces[3];
    // cout<<"my_total is "<<my_total<<endl;
    int opp_total = opp_pieces[0]+opp_pieces[1]+opp_pieces[2]+opp_pieces[3];
    // cout<<"opp_total is "<<opp_total<<endl;
    // if(my_pieces[0]==0 || opp_pieces[0]==0){
    //     if(my_pieces[0]==0){
    //         return (-4*(my_total-opp_total)+4)*(-60);
    //     }
    //     else if(opp_pieces[0]==0){
    //         return (-4*(my_total-opp_total)+4)*(60);
    //     }
    // }
    // if(b.in_check()==true){
    //     bool flag = false;
    //     for(auto i:next_moves){
    //         if(i.second->in_check()==false){ 
    //             flag = true;
    //             break;
    //         }
    //     }
    //     if(flag==false){
    //         return 
    //     }
    // }
    if(d==0){
        // cout<<"in d==0\n";
        double e = evaluation(rb);
        // cout<<" evaluated val is "<<e<<" parent val is "<<parent_val<<endl;
        // if(e-parent_val>100 || e-parent_val<100){
        //     return max_quisecent(rb,2,parent_val);
        // }
        // else{
        //     return e;
        // }
        return e;
    }
    if(rb->children.size()==0){
        // cout<<"-------calling create states------"<<endl;
        rb->create_states();
        // cout<<"---------after create states-------"<<endl;
    }
    if(rb->children.size()==0){
        // cout<<" Before return in 0 size"<<endl;
        return ((my_total-opp_total)*4 -2)*6000;
    }
    // cout<<"children states created\n";
    double v = numeric_limits<double>::min();  // minimum value
    double temp = 0;
    unsigned long long int h;
    for(auto i:rb->children){
        h = get_hash(i->value_matrix);
        // cout<<"max_hash is "<<h<<endl;
        if(transposition_table.find(h)==transposition_table.end()){
            // cout<<"Before min_val call"<<endl;
            temp = min_val(i,alpha,beta,d-1,start,time_limit,parent_val);
            // cout<<"After min_val "<<temp<<endl;
        }
        else{
            // cout<<"No need to call min_val"<<endl;
            temp = transposition_table.at(h);
            // cout<<"temp is "<<temp<<endl;
            // cout<<"In else"<<endl;
        }
        v = max(v,temp);
        parent_val = v;
        // cout<<"Before eval_val"<<endl;
        i->eval_val = temp;
        // cout<<"After eval_val "<<rb.eval_val<<endl;
        // if(temp>numeric_limits<double>::min() && temp<numeric_limits<double>::max()) transposition_table[h]=temp;
        transposition_table[h]=temp;
        if(v>=beta){
            sort(rb->children.begin(),rb->children.end(),max_compare);
            // cout<<"---------Beta prunes-------------------"<<endl;
            return v;
        }
        // cout<<"Before alpha"<<endl;        
        alpha = max(alpha,v);
    }
    sort(rb->children.begin(),rb->children.end(),max_compare);
    return v;
}

double min_val(RollerBoard* rb, double alpha, double beta, int d, chrono::time_point<chrono::system_clock>start, double time_limit, double parent_val){
    chrono::duration<double> elapsed_time = chrono::system_clock::now()-start;
    // cout<<"min_Begin\n";
    if (elapsed_time.count()>time_limit){
        TimeException t;
        throw t;
    }
    vector<int> my_pieces = get_my_pieces(rb);
    vector<int> opp_pieces = get_opp_pieces(rb);
    int my_total = my_pieces[0]+my_pieces[1]+my_pieces[2]+my_pieces[3];
    // cout<<"min_my_total is "<<my_total<<endl;
    int opp_total = opp_pieces[0]+opp_pieces[1]+opp_pieces[2]+opp_pieces[3];
    // cout<<"min_opp_total is "<<opp_total<<endl;
    // if(my_pieces[0]==0 || opp_pieces[0]==0){
    //     if(my_pieces[0]==0){
    //         // cout<<"In constant function"<<endl;
    //         return (-4*(my_total-opp_total)+4)*(-6000);
    //     }
    //     else if(opp_pieces[0]==0){
    //         // cout<<"In else constant function"<<endl;
    //         return (-4*(my_total-opp_total)+4)*(6000);
    //     }
    // }
    // if(b.in_check()==true){
    //     bool flag = false;
    //     for(auto i:next_moves){
    //         if(i.second->in_check()==false){ 
    //             flag = true;
    //             break;
    //         }
    //     }
    //     if(flag==false){
    //         return 
    //     }
    // }
    if(d==0){
        // cout<<"min_in d==0\n";
        double e = evaluation(rb);
        // cout<<"min evaluated val is "<<e<<" parent_val is "<<parent_val<<endl;
        // cout<<"after evaluation "<<e<<endl;
        // if(e-parent_val>100 || e-parent_val<100){
        //     // cout<<"before min_q"<<endl;
        //     return min_quisecent(rb,2,parent_val);
        //     // cout<<"after min_q"<<endl;
        // }
        // else{
        //     return e;
        // }
        return e;
    }
    if(rb->children.size()==0){
        rb->create_states();
    }
    if(rb->children.size()==0){
        // cout<<"child states is zero"<<endl;
        return ((my_total-opp_total)*4 -2)*6000;
    }
    // cout<<"min_children states created\n";
    double v = numeric_limits<double>::max();  // minimum value
    double temp = 0;
    unsigned long long int h;
    for(auto i:rb->children){
        h = get_hash(i->value_matrix);
        // cout<<"min_hash is "<<h<<endl;
        if(transposition_table.find(h)==transposition_table.end()){
            // cout<<"min_Before max_val"<<endl;
            temp = max_val(i,alpha,beta,d-1,start,time_limit,parent_val);
            // cout<<"min_After max_val "<<temp<<endl;
        }
        else{
            // cout<<"No need to call max_val"<<endl;
            temp = transposition_table.at(h);
            // cout<<"Temp_max is "<<temp<<endl;
            // cout<<"min_In else"<<endl;
        }
        v = min(v,temp);
        parent_val = v;
        // cout<<"min_Before eval_val"<<endl;
        i->eval_val = temp;
        // cout<<"min_After eval_val "<<rb.eval_val<<endl;
        // if(temp>numeric_limits<double>::min() && temp<numeric_limits<double>::max()) transposition_table[h]=temp;
        transposition_table[h]=temp;
        if(v<=alpha){
            sort(rb->children.begin(),rb->children.end(),min_compare);
            // cout<<"------------Alpha prunes----------------------"<<endl;
            return v;
        }
        beta = min(beta,v);
    }
    sort(rb->children.begin(),rb->children.end(),min_compare);
    return v;
}

U16 alpha_beta_search(RollerBoard* b, int d, chrono::time_point<chrono::system_clock> start,double time_limit){
    // cout<<"in alpha beta-----------------------------------"<<endl;
    int value = 0;
    double maxi = numeric_limits<double>::max();  // maximum value
    double mini = numeric_limits<double>::min();  // minimum value
    value = max_val(b,mini,maxi,d,start,time_limit,mini);
    b->eval_val = value;
    if(b->children.size()>0) return b->children[0]->moves.at(b->children[0]->moves.size()-1);
    abort();   
}

U16 iterative_deepening(RollerBoard* b, double time_left, double total_time){
    initTable();

    int depth = 10;
    int d = 1;
    double time_limit = 2;
    U16 best_move = 0;
    // if(time_left>total_time-total_time/8) time_limit=1.25;
    // else if(time_left>total_time-total_time/4) time_limit=1.75;
    // else if(time_left>total_time/4) time_limit=2;
    // else if(time_left>total_time/8) time_limit=1.25;
    // else time_limit = 1;
    chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
    try{
        while(d<depth){
            // cout<<"started with depth "<<d<<endl;
            transposition_table.erase(transposition_table.begin(),transposition_table.end());
            best_move = alpha_beta_search(b,d,start,time_limit);
            d++;
        }
    }
    catch(const exception& TimeException){
        return best_move;
    }
    return best_move;
}

void Engine::find_best_move(const Board& b) {
    if(b.data.player_to_play == WHITE){ current_player = true;}
    RollerBoard* rb = new RollerBoard(b);
    this->best_move = iterative_deepening(rb,2,2);
    // chrono::time_point<chrono::system_clock> start = chrono::system_clock::now();
    // this->best_move = alpha_beta_search(rb,4,start,2);
}