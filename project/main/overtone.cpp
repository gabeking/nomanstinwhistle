//Necessary Includes
#include <SDL2/SDL.h>			//Main SDL header
#include <SDL2/SDL_image.h>		//SDL image loading/moving
#include <SDL2/SDL_ttf.h>		//SDL text placement
#include <SDL2/SDL_mixer.h>		//SDL Music Player
#include <stdio.h>				//standard input/output / filestreams
#include <string>				//C++ string library
#include <vector>				//C++ vector library
#include <fstream>				//File i/o
#include <stdlib.h>				//C general utilities (rand, atoi, etc.)
#include <iostream>				//cin, cout, etc
#include <list> 				//STL list
#include <random>
using namespace std;			//STL namespace

#include "./classes/enemy.h"
#include "./classes/texture.h"			//Sets textures to objects
#include "./classes/sdlTools.h"			//SDL utilities
#include "./classes/note.h"				//Holds note class for enemy spawning
#include "./classes/sprite.h"				//For general movement, spawning, etc
#include "./classes/player.h"				//Player specific object code
#include "./classes/laser.h"
#include "./classes/stars.h"
#include "./classes/collisions.h"          //Function checks collisons between
                     			           //Two sprites

//Function prototypes
vector<note> set_up_music_adt();		//Takes txt files and converts them
										//to note class

//Global Variables
string PROGRAM_NAME;	//Holds program name, for usage

int SCORE = 0;
int SCORE_INCREMENT = 5;
int LIVES = 3;

int MULTIPLIER = 1;
int STREAK = 0;

SDL_Color textColor = { 255, 255, 255 };

int DIFFICULTY = 2; 		//Game Difficulty (1: "easy", 2: "medium", or 3: "hard")

string SONGNAME = "one";	//Name of song that user wishes to use
							//will be one, two, or three

void usage(int status) {	//Prints if '-h' argument is used
	cout << "Usage: ./" << PROGRAM_NAME << "DIFFICULTY SONGFILE" << endl;
	cout << "DIFFICULTY can be 1, 2, or 3" << endl;
	cout << "SONGFILE can be one, two, or three" << endl;
    exit(status);
}




// screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Necessary implementation variables for SDL
bool loadMedia();

SDL_Window* gWindow = NULL;

SDL_Renderer* gRenderer = NULL;

TTF_Font *gFont = NULL;

texture gTextTexture;
texture gTextTextureLives;
texture gTextTextureMultiplier;
texture gTextTexturePowerUp;

Mix_Music *gMusic = NULL;

const int SCREEN_FPS = 60;
const int SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS;

SDL_Rect playerSpriteClipArray[8];
SDL_Rect laserSpriteClipArray[1];
std::vector<SDL_Rect*> laserSpriteClips;
SDL_Rect gSpriteClipArray[4];
std::vector<SDL_Rect*> playerSpriteClips;
std::vector<texture> gSpriteSheetTextures;
SDL_Rect enemySpriteClipArray[6];
std::vector<SDL_Rect*> enemySpriteClips;

bool loadMedia();   // loads files for sprite textures

//Main function
int main( int argc, char* argv[] )
{
	//Command line argument parsing
	int argind = 1;
    PROGRAM_NAME = argv[0];
    while (argind < argc) {
        char* arg = argv[argind++];
		if (strcmp(arg, "-h")==0){
			usage(0);
		}
    	if (argind == 2){	//Checks user-inputted DIFFICULTY
			if (strcmp(arg, "1")==0 || strcmp(arg, "2")==0 || strcmp(arg, "3")==0){
				DIFFICULTY = atoi(arg);
			}else{
				cout << "DIFFICULTY input not understood, defaulting to medium." << endl;
			}
		}
		if (argind == 3){	//Checks user-inputted SONGNAME
			if (strcmp(arg, "one")==0 || strcmp(arg, "two")==0 || strcmp(arg, "three")==0){
				SONGNAME = arg;
			}else{
				cout << "SONGNAME input not understood, defaulting to one." << endl;
			}
		}
    }
	cout << "DIFFICULTY is: " << DIFFICULTY << endl;
	cout << "SONGNAME is: " << SONGNAME << endl;	

	srand (time(NULL));		//Seeds random number generator

	
	vector<note> song_notes = set_up_music_adt();		//Calls adt generation
	
	
    //Start up SDL and create window
    if( !init(gWindow, gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT) )
    {   
        printf( "Failed to initialize!\n" );
    }
    else
    {   
        texture texture0;

        gSpriteSheetTextures.push_back(texture0);
        gSpriteSheetTextures.push_back(texture0);
        gSpriteSheetTextures.push_back(texture0);
		gSpriteSheetTextures.push_back(texture0);
		gSpriteSheetTextures.push_back(texture0);
		gSpriteSheetTextures.push_back(texture0);
        //Load media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
            // Seed random number generator
            srand(time(NULL));

            // Initialize fps cap timer
            SDLTimer capTimer;

            //Initialize player
            player Player(20, SCREEN_HEIGHT/2, 
            SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[0]);
            Player.setClips(playerSpriteClips);
            
            // initialize backgrounds
            stars Stars(SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[2]);
		
			//Initialize top bar
			sprite TopBar (SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[4]);

			//Initialize bottom bar
			sprite BottomBar (SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[5]);
			BottomBar.setPos(0, 440);

            //Main loop flag
            bool quit = false;

            //Event handler
            SDL_Event e;

            SDLTimer laser_timer;

            laser_timer.start();

			SDLTimer enemy_timer;
			
            // start enemy timer
			enemy_timer.start();
	        
            // timer for player invulnerability after being hit
            SDLTimer player_timer;
            player_timer.start();
            unsigned int player_coolDown = 1000;

            // time between laser shots (in ms)
            unsigned int laser_coolDown = 60;

			double songLength = song_notes.back().getOnset();
			
            list<laser> laser_list;
            int laserSpeed = 10;

			list<enemy> enemy_list;
            
            // Animation counter = number of frames per change in sprite
            int animCounterMax = 5;
            int animCounter = 0;

			// Color Change Counter
			int colorCounterMax = 10;
			int colorCounter = 0;            

            //While application is running
            while( !quit )
            {
                
                // start cap timer
                capTimer.start();
		

                //Handle events on queue
                while( SDL_PollEvent( &e ) != 0 ) 
                {
                    //User requests quit
                    if( e.type == SDL_QUIT )
                    {
                        quit = true;
                    }
                }

                //Clear screen
                SDL_SetRenderDrawColor( gRenderer, 0x00, 0x00, 0x03, 0xFF );
                SDL_RenderClear( gRenderer );
                
                // render background stars
                Stars.update();

                // Handle player sprite
                animCounter++;
                if (animCounter >= animCounterMax) {
                    Player.nextClip();
                }

                Player.update();
                
                if ((player_timer.getTicks() > player_coolDown) || (player_timer.getTicks() % 150 < 80))
                Player.render();

                // Spawn lasers
                const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
                if (keyStates[SDL_SCANCODE_SPACE]) {
                    // check if cooldown period has passed
                    if (laser_timer.getTicks() > laser_coolDown) {
                        laser_timer.start();
                        
                        // add new laser to laser vector
                        laser new_laser1(laserSpeed, Player.getYVel(), SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[1]);
                        new_laser1.setClips(laserSpriteClips);
                        new_laser1.setPos(Player.getX() + Player.getWidth() - new_laser1.getWidth()/2, Player.getMidY() - new_laser1.getHeight()/2);
                        laser_list.push_back(new_laser1);
						
                        if (STREAK >= 50){
                            // load triple lasers for higher streaks
							gTextTexturePowerUp.loadFromRenderedText(gFont, "Power Up: TriLaser", textColor );
							laser new_laser2(laserSpeed, Player.getYVel()-1, SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[1]);
		                    new_laser2.setClips(laserSpriteClips);
		                    new_laser2.setPos(Player.getX() + Player.getWidth() - new_laser2.getWidth()/2, Player.getMidY() - new_laser2.getHeight()/2);
		                    laser_list.push_back(new_laser2);
							laser new_laser3(laserSpeed, Player.getYVel()+1, SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[1]);
		                    new_laser3.setClips(laserSpriteClips);
		                    new_laser3.setPos(Player.getX() + Player.getWidth() - new_laser3.getWidth()/2, Player.getMidY() - new_laser3.getHeight()/2);
		                    laser_list.push_back(new_laser3);
						}
                    }
                }
                
                // check laser collisons
                list<laser>::iterator it;
                for (it = laser_list.begin(); it != laser_list.end();) {
                    bool erased = false;
                    it->update();
                    it->render();
                    list<enemy>::iterator itE;
                    for (itE = enemy_list.begin(); itE != enemy_list.end();) { 
                        if (checkColl(&(*it), &(*itE))) {
                            it = laser_list.erase(it);
                            itE = enemy_list.erase(itE);
							STREAK+=1;
							if (STREAK <= 10){
								MULTIPLIER = 1;
							}else if (STREAK <=20){
								MULTIPLIER = 2;
							}else{
								MULTIPLIER = 3;
							}
							string multiplies = "Multiplier: " + to_string(MULTIPLIER);
							gTextTextureMultiplier.loadFromRenderedText(gFont, multiplies, textColor );
                            erased = true;
							SCORE += SCORE_INCREMENT * MULTIPLIER;
							string score_string_raw = to_string(SCORE);
							string score_string = "0000000";
							score_string.replace(score_string.length()-score_string_raw.length(), score_string_raw.length(), score_string_raw);
							gTextTexture.loadFromRenderedText(gFont, score_string, textColor );
                            break;
                        }
                        itE++;
                    }
                    if (!erased && !it->isOnScreen())
                        it = laser_list.erase(it);
                    else 
                        it++;
                }

				//check if enemy wants to be spawned
				if (song_notes.size() > 1){
					if (enemy_timer.getTicks() > song_notes[0].getOnset()*1000 ){
						enemy new_enemy(song_notes[0], SCREEN_WIDTH, SCREEN_HEIGHT, &gSpriteSheetTextures[3]);
						new_enemy.setClips(enemySpriteClips);
						enemy_list.push_back(new_enemy);
						if (song_notes.size() > 0){
							song_notes.erase(song_notes.begin());
						}			
					}
				}

				// update enemy color based on song frequency
                colorCounter++;
				if (song_notes.size() > 0){
					if (colorCounter >= colorCounterMax){
						colorCounter=0;
						int color_freq = song_notes[0].getFreq();
						double red_rand = (double)(rand() % 100) / 50;
						double green_rand = (double)(rand() % 100) / 50;
						double blue_rand = (double)(rand() % 100) / 50;
						int red = 255*(red_rand*(double)color_freq/500);
						int green = 255-(255*((double)color_freq/1000)*green_rand);
						int blue = 255-(255*((double)color_freq/500))*blue_rand;
						gSpriteSheetTextures[3].setColor(red, green, blue);
					}
				}
				if (enemy_timer.getTicks() > (songLength+5)*1000){
					break;
				}
				
				// update enemies
                list<enemy>::iterator iterator;
				for (iterator = enemy_list.begin(); iterator != enemy_list.end();){
					iterator->update();
                    if (animCounter >= animCounterMax) {
                        iterator->nextClip();
                    }
					iterator->render();
                    // check collision with player
                    if ((player_timer.getTicks() > player_coolDown) && checkColl(&Player, &(*iterator))) {
						player_timer.start();
                        LIVES--;						
						string lives = "Lives: " + to_string(LIVES);
						gTextTextureLives.loadFromRenderedText(gFont, lives, textColor );
						STREAK = 0;
						MULTIPLIER = 1;
						string multiplies = "Multiplier: " + to_string(MULTIPLIER);
						gTextTexturePowerUp.loadFromRenderedText(gFont, "Power Up: None", textColor );
						gTextTextureMultiplier.loadFromRenderedText(gFont, multiplies, textColor );
                        iterator = enemy_list.erase(iterator);
                    }
                    else {
                        ++iterator;
                    }
				}

				if( Mix_PlayingMusic() == 0 ) { 
					Mix_PlayMusic( gMusic, -1 ); 
				}	

				// Render UI bars on top and bottom of window
                BottomBar.render();
				TopBar.render();
				gTextTexture.render( 10, 3);				
				gTextTextureLives.render(575, 3);
				gTextTextureMultiplier.render(10, 460);
				gTextTexturePowerUp.render(500,460);
                
                // Reset animation counter
                if (animCounter >= animCounterMax) {
                    animCounter = 0; 
                }

                //Update screen
                SDL_RenderPresent( gRenderer );

                // if frame finished early
                Uint32 frameTicks = capTimer.getTicks();
                if (frameTicks < SCREEN_TICKS_PER_FRAME) {
                    // wait remaining time
                    SDL_Delay( SCREEN_TICKS_PER_FRAME - frameTicks);
                }
			
				// end game when player has no remaining lives
                if (LIVES < 1){
					break;
				}
            }
        }
    }

    // end game state
	Mix_HaltMusic();	
	SDL_RenderClear( gRenderer );
	
    const Uint8 *keyStates = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	bool quit = false;
	while (!keyStates[SDL_SCANCODE_Q] && !quit){
		while (SDL_PollEvent(&e) !=0){
			 //User requests quit
             if( e.type == SDL_QUIT )
             {
                 quit = true;
             }
		}
		// display user score
        string score_string_raw = to_string(SCORE);
		string score_string = "0000000";
		score_string.replace(score_string.length()-score_string_raw.length(), score_string_raw.length(), score_string_raw);
		score_string = "Final Score: " + score_string;
		gTextTexture.loadFromRenderedText(gFont, score_string, textColor );
		gTextTexture.render(230,200);
		gTextTextureLives.loadFromRenderedText(gFont, "Press Q to End Game...", textColor );
		gTextTextureLives.render(230, 250);
		SDL_RenderPresent(gRenderer);
	}
	
	
	// Free resources and close SDL
    close(gMusic, gFont, gWindow, gRenderer, gSpriteSheetTextures);
	
    return 0;
}

bool loadMedia()
	{
    //Loading success flag
    bool success = true;

    //Load Player texture
    if( !gSpriteSheetTextures[0].loadFromFile( "./assets/playershipv2.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" );
        success = false;
    }
    else
    {
        // Set player clipping rectangles
        playerSpriteClipArray[0].x = 0;
        playerSpriteClipArray[0].y = 0;
        playerSpriteClipArray[0].w = 51;
        playerSpriteClipArray[0].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[0]);

        playerSpriteClipArray[1].x = 51;
        playerSpriteClipArray[1].y = 0;
        playerSpriteClipArray[1].w = 51;
        playerSpriteClipArray[1].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[1]);

        playerSpriteClipArray[2].x = 0;
        playerSpriteClipArray[2].y = 20;
        playerSpriteClipArray[2].w = 51;
        playerSpriteClipArray[2].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[2]);

        playerSpriteClipArray[3].x = 51;
        playerSpriteClipArray[3].y = 20;
        playerSpriteClipArray[3].w = 51;
        playerSpriteClipArray[3].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[3]);

        playerSpriteClipArray[4].x = 0;
        playerSpriteClipArray[4].y = 40;
        playerSpriteClipArray[4].w = 51;
        playerSpriteClipArray[4].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[4]);

        playerSpriteClipArray[5].x = 51;
        playerSpriteClipArray[5].y = 40;
        playerSpriteClipArray[5].w = 51;
        playerSpriteClipArray[5].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[5]);

        playerSpriteClipArray[6].x = 0;
        playerSpriteClipArray[6].y = 60;
        playerSpriteClipArray[6].w = 51;
        playerSpriteClipArray[6].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[6]);

        playerSpriteClipArray[7].x = 51;
        playerSpriteClipArray[7].y = 60;
        playerSpriteClipArray[7].w = 51;
        playerSpriteClipArray[7].h = 20;
        playerSpriteClips.push_back(&playerSpriteClipArray[7]);
    }
    
    // Load laser texture 
    if( !gSpriteSheetTextures[1].loadFromFile( "./assets/laser.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" );
        success = false;
    }
    else {
        // set laser clipping rectangles
        laserSpriteClipArray[0].x =  0;
        laserSpriteClipArray[0].y =  0;
        laserSpriteClipArray[0].w = 25;
        laserSpriteClipArray[0].h =  7;
        laserSpriteClips.push_back(&laserSpriteClipArray[0]);
    }


	// Load enemy texture
    if( !gSpriteSheetTextures[3].loadFromFile( "./assets/alienshipv2.png"))
    {
        printf( "Failed to load sprite sheet texture!\n" );
        success = false;
    }
    else {
        // set enemy cliiping rectangles
        enemySpriteClipArray[0].x = 0;
        enemySpriteClipArray[0].y = 0;
        enemySpriteClipArray[0].w = 32;
        enemySpriteClipArray[0].h = 22;
        enemySpriteClips.push_back(&enemySpriteClipArray[0]);
    
        enemySpriteClipArray[1].x = 32;
        enemySpriteClipArray[1].y = 0;
        enemySpriteClipArray[1].w = 32;
        enemySpriteClipArray[1].h = 22;
        enemySpriteClips.push_back(&enemySpriteClipArray[1]);
    
        enemySpriteClipArray[2].x = 0;
        enemySpriteClipArray[2].y = 22;
        enemySpriteClipArray[2].w = 32;
        enemySpriteClipArray[2].h = 22;
        enemySpriteClips.push_back(&enemySpriteClipArray[2]);
    
        enemySpriteClipArray[3].x = 32;
        enemySpriteClipArray[3].y = 22;
        enemySpriteClipArray[3].w = 32;
        enemySpriteClipArray[3].h = 22;
        enemySpriteClips.push_back(&enemySpriteClipArray[3]);
    
        enemySpriteClipArray[4].x = 0;
        enemySpriteClipArray[4].y = 22;
        enemySpriteClipArray[4].w = 32;
        enemySpriteClipArray[4].h = 22;
        enemySpriteClips.push_back(&enemySpriteClipArray[4]);
    
        enemySpriteClipArray[5].x = 32;
        enemySpriteClipArray[5].y = 0;
        enemySpriteClipArray[5].w = 32;
        enemySpriteClipArray[5].h = 22;
        enemySpriteClips.push_back(&enemySpriteClipArray[5]);
    
    }


    // load star texture
    if( !gSpriteSheetTextures[2].loadFromFile( "./assets/star.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" );
        success = false;
    }

	// load UI bar texture
    if( !gSpriteSheetTextures[4].loadFromFile( "./assets/bartop.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" );
        success = false;
    }

	// load UI bar texture
    if( !gSpriteSheetTextures[5].loadFromFile( "./assets/barbottom.png" ) )
    {
        printf( "Failed to load sprite sheet texture!\n" );
        success = false;
    }


	//Open the font 
	gFont = TTF_OpenFont( "fonts/zekton.ttf", 15 ); 
	if( gFont == NULL ) { 
		printf( "Failed to load font! SDL_ttf Error: %s\n", TTF_GetError() ); 
		success = false; 
	} else { //Render text SDL_Color 
		//SDL_Color textColor = { 0, 0, 0 }; 
		if( !gTextTexture.loadFromRenderedText(gFont, "0000000", textColor ) ) { 
			printf( "Failed to render text texture!\n" ); 
			success = false; 
		} 
		string lives_text = "Lives: " + to_string(LIVES);
		if( !gTextTextureLives.loadFromRenderedText(gFont, lives_text, textColor ) ) { 
			printf( "Failed to render text texture!\n" ); 
			success = false; 
		}
		string multiplier_text = "Multiplier: " + to_string(MULTIPLIER);
		if( !gTextTextureMultiplier.loadFromRenderedText(gFont, multiplier_text, textColor ) ) { 
			printf( "Failed to render text texture!\n" ); 
			success = false; 
		} 
		if( !gTextTexturePowerUp.loadFromRenderedText(gFont, "Power Up: None", textColor ) ) { 
			printf( "Failed to render text texture!\n" ); 
			success = false; 
		} 
	}
	
	//Load music 
	SONGNAME = "./songfiles/" + SONGNAME;
	gMusic = Mix_LoadMUS( SONGNAME.c_str() ); 
	if( gMusic == NULL ) { 
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() ); 
		success = false; 
	}

    return success;
}


vector<note> set_up_music_adt(){
	
	/*
	 * This will intitialize the note class as a vector of class notes
	 */
	vector<note> total_notes;
	vector<note> song_notes;
	vector<double> time_vec;
	vector<double> freq_vec;
	string input_string; 
	
	//Takes the numbers from the times.txt and 
	//inputs them into a vector
	string timefilename = "./songfiles/" + SONGNAME + "time.txt";
	ifstream timefile(timefilename);
    while (std::getline(timefile, input_string))
    {
        time_vec.push_back(stod(input_string));
    }
    
    //Takes the numbers from the notes.txt and
    //inputs them into a vector
    string freqfilename = "./songfiles/" + SONGNAME + "note.txt";
	ifstream freqfile(freqfilename);
    while (std::getline(freqfile, input_string))
    {
        freq_vec.push_back(stod(input_string));

    }
    
    //Pushes the times and frequencies into a vector of note classes.
    for (unsigned int counter = 0; counter < freq_vec.size() && counter < time_vec.size(); ++counter){
		note new_note(SCREEN_HEIGHT, freq_vec[counter], time_vec[counter]);
		total_notes.push_back(new_note);
	}
	//second and second_increment are used to 
	//take out a certain amount of notes from the total_notes vector
	double second;
	double second_increment;

	
	//This changes that amount of enemies that will be spawned
	//based on the difficulty
	switch (DIFFICULTY) {
		case 1:
			second = 0.25;
			second_increment = 0.25;
			break;
		case 2:
			second = 0.125;
			second_increment =0.125;
			break;
		case 3:
			second = 0.03125;
			second_increment = 0.03125;
			break;
		default:
			second = 3;
			second_increment = 3;
			break;
	}

	//lower_size and upper_size will keep the range from which
	//the note will be drawn
	int lower_size = 0;
	int upper_size = 0;
	int note_to_choose = 0;
	


	//This for loop takes one note, at random, in every 1, 2, or 3 
	//second range
	for (unsigned int counter = 0; counter < total_notes.size(); ++counter){
		if (total_notes[counter].getOnset() > second){
			//cout << "second: " << second << " onset: " << total_notes[counter].getOnset() << " counter: " << counter << endl;
			int modulo = upper_size-lower_size;
			note_to_choose = rand() % modulo + lower_size - 1;
			song_notes.push_back(total_notes[note_to_choose]);
			//cout << "chosen index: " << note_to_choose << " chosen onset: " << total_notes[note_to_choose].getOnset() << endl;
			second+=second_increment;
			lower_size=upper_size;
		}
		upper_size++;
	}
	
	//for (int i = 0; i < song_notes.size(); ++i){
	//	cout << song_notes[i].getOnset() << endl;

	//}
	
	return song_notes;
}
