 /* song.h
 * implementation for song object for CSE20312 project
 * Jeff Klouda
 */

// NOTES
// consider alternate data types for "notes" as we have a better idea what we
//      need from it
// Modify nextNote() or create new function to find last note in song

#ifndef SONG_H
#define SONG_H

#include <queue>
#include <string>
#include <iostream>
#include "note.h"
#include "song.h"

using namespace std;

struct song {
  song() {
    // we probably don't want to use the default constructor
    cout << "song default constructor called\n";
    title = "";
    composer = "";
    meter1 = 4;
    meter2 = 4;
    key = "";
    tempo = 60;
  }
  song(string title, string composer, int m1, int m2, string key, float tempo) {
    setTitle(title);
    setComposer(composer);
    setM1(m1);
    setM2(m2);
    setKey(key);
    setTempo(tempo);
  }
  ~song() {};


  void addNote(note newNote) {
    notes.push(newNote);
  }

  note nextNote() { // add condition to catch last note
    note returnNote = notes.front();
    notes.pop();
    return returnNote;
  }

  void setTitle(string title) {
    this->title = title;
  }

  void setComposer(string composer) {
    this->composer = composer;
  }

  void setM1(int m1) {
    this->meter1 = m1;
  } 

  void setM2(int m2) {
    this->meter2 = m2;
  }

  void setKey(string key) {
  this->key = key;
  }

  void setTempo(float tempo) {
    this->tempo = tempo;
  }

  // display all data in song
  void display() { // for debugging purposes
    cout << "Title: " << title << endl;
    cout << "Composer: " << composer << endl;
    cout << "Meter: " << meter1 << "/" << meter2 << endl;
    cout << "Key: " << key << endl;
    cout << "Tempo: " << tempo << endl;
    // display note letters
    note nNote;
    while (!notes.empty()) {
      nNote = this->nextNote();
      cout << nNote.getLetter() << nNote.getOctave() << "-"
        << nNote.getLength() << " ";
    }
    cout << endl;
  }

  string title;
  string composer;
  int meter1; // top number of meter (beats per measure)
  int meter2; // lower number of meter (which note gets the beat)
  string key; // python script handles changes to notes based on key
  float tempo; // in beats per minute
  queue<note> notes; // contains all notes in song
};

#endif
