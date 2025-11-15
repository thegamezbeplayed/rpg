#include <stdio.h>
#include "game_utils.h"
#include "raylib.h"
#include "game_assets.h"

static audio_manager_t AudioMan;
MAKE_ADAPTER(AudioPlayNext, music_group_t*);
MAKE_ADAPTER(AudioMusicFade, music_track_t*);

void InitAudio(void){
  AudioMan = (audio_manager_t){0};
  char fullpath[256]; // make sure it's large enough

  AudioMan.current_track = -1;
  AudioMan.concurrent_track = -1;
  //FilePathList audiofiles = LoadDirectoryFiles("resources/sfx/action");
  for(int i = 0; i < SFX_DONE; i++){
    AudioMan.sfx[i] = (sfx_group_t){0};
    AudioMan.timers[i] = InitEvents();
    AudioMan.sfx[i].volume = 0.125f;
  }

  for (int i = 0; i < END_SFX; i++){
    sfx_info_t info = sfx_catalog[i];
    AudioMan.sfx[info.group].num_types++;

    AudioMan.sfx[info.group].items[i] =(sfx_sound_t) {0};
    AudioMan.sfx[info.group].items[i].type =info.type;
    AudioMan.sfx[info.group].items[i].num_sounds = info.num_sounds;

    AudioMan.sfx[info.group].items[i].sounds = calloc(info.num_sounds,sizeof(Sound));
    for (int j = 0; j < info.num_sounds; j++){
      sprintf(fullpath, "resources/sfx/%s", info.paths[j]);
      AudioMan.sfx[info.group].items[i].sounds[j] = LoadSound(fullpath);

      TraceLog(LOG_INFO,"Found sfx file %s",fullpath);
    }
  }
}

int AudioBuildMusicTracks(const char* subdir){
  char fullpath[256]; // make sure it's large enough
  sprintf(fullpath, "resources/music/%s", subdir);
  FilePathList musicfiles = LoadDirectoryFiles(fullpath);
  music_group_t *album = &AudioMan.tracks[AudioMan.num_tracks];
  *album = (music_group_t){0};

  album->volume = 0.75f;
  album->index = AudioMan.num_tracks;

  album->num_songs = musicfiles.count;

  int currentIndex = 1;
  for(int i = 0; i< musicfiles.count; i++){
    const char *filename = GetFileName(musicfiles.paths[i]);  // e.g. "tut_loop_01.ogg"
    char *stem = GetFileStem(filename);            // e.g. "tut_loop_01"
    music_track_t track = {0};
    track.music = LoadMusicStream(musicfiles.paths[i]);
    track.events = InitEvents(); 

    track.fade = malloc(sizeof(audio_fade_t));
    track.fade->vol = 1.0f; 
    if (strstr(stem, "intro")){
      cooldown_t *end = InitCooldown((int)(track.music.frameCount*60/track.music.stream.sampleRate)-1,EVENT_SONG_END,AudioPlayNext_Adapter,album);
      end->is_recycled = true;
      AddEvent(track.events,end);
track.music.looping = false;
      album->track[0] = track;
    }
    else{
      track.music.looping = true;
      album->track[currentIndex]=track;
      currentIndex++;
    }
  }

  return AudioMan.num_tracks++;
}

void AudioPlayMusic(int index){
  if(index < 0)
    return;
  
  int curTrack = AudioMan.current_track;
  if(curTrack != index && curTrack >= 0){
    music_track_t *g = &AudioMan.tracks[curTrack].track[AudioMan.tracks[curTrack].current_index];
    g->fade->increase = false;
    g->fade->duration = 180;
    g->fade->vol = 1.0f;
    g->fade->elapsed = 0;

    cooldown_t *fOut = InitCooldown(180,EVENT_SONG_FADE_OUT,NULL,NULL);
    fOut->on_step = AudioMusicFade_Adapter;
    fOut->on_step_params = g;
    AddEvent(g->events,fOut);
    AudioMan.concurrent_track = curTrack;
    music_track_t *n = &AudioMan.tracks[index].track[0];
    n->fade->increase = true;
    n->fade->duration = 300;
    n->fade->vol = 0.0f;
    n->fade->elapsed = 0;

    AudioMan.current_track = index;
    cooldown_t *fIn = InitCooldown(300,EVENT_SONG_FADE_IN,NULL,NULL);
    fIn->on_step = AudioMusicFade_Adapter;
    fIn->on_step_params = n;
    AddEvent(n->events,fIn);

    music_group_t s = AudioMan.tracks[index];
    PlayMusicStream(s.track[s.current_index].music);
  }
  else{
    music_group_t g = AudioMan.tracks[index];
    AudioMan.current_track = index;
    PlayMusicStream(g.track[g.current_index].music);
  }
}

bool AudioPlayNext(music_group_t* t){
  ResetEvent(t->track[t->current_index].events,EVENT_SONG_END);
  t->current_index++;
  if(t->current_index >= t->num_songs)
    t->current_index = 1;

  AudioPlayMusic(t->index);

  return true;
}

void AudioMusicFade(music_track_t* t){
  t->fade->elapsed++;
  float change = 1.0f * t->fade->elapsed / t->fade->duration;
  if(!t->fade->increase)
    change =1.0f - change;

  t->fade->vol = change;

  if(t->fade->increase && t->fade->elapsed >= t->fade->duration)
    AudioMan.concurrent_track = -1;
}

void AudioPlaySfx(SfxGroup group, SfxType type, int index){
  if(CheckEvent(AudioMan.timers[group],EVENT_PLAY_SFX))
    return;


    PlaySound(AudioMan.sfx[group].items[type].sounds[index]);
    SetSoundVolume(AudioMan.sfx[group].items[type].sounds[index],AudioMan.sfx[group].volume);
    int wait  = (int)((AudioMan.sfx[group].items[type].sounds[index].frameCount*30)/44100);
    AddEvent(AudioMan.timers[group],InitCooldown(wait,EVENT_PLAY_SFX,NULL,NULL));
}

void AudioPlayRandomSfx(SfxGroup group, SfxType type){
  if(CheckEvent(AudioMan.timers[group],EVENT_PLAY_SFX))
    return;

  for(int i = 0; i < AudioMan.sfx[group].num_types; i++){
    if(AudioMan.sfx[group].items[i].type != type)
      continue;

    int r = rand()%AudioMan.sfx[group].items[i].num_sounds;
    PlaySound(AudioMan.sfx[group].items[i].sounds[r]);
    SetSoundVolume(AudioMan.sfx[group].items[i].sounds[r],AudioMan.sfx[group].volume);
    int wait  = (int)((AudioMan.sfx[group].items[i].sounds[r].frameCount*30)/44100);
    AddEvent(AudioMan.timers[group],InitCooldown(wait,EVENT_PLAY_SFX,NULL,NULL));
    return;
  }
}

void AudioStep(){
  for(int g = 0; g<SFX_DONE;g++){
    for(int i = 0; i< AudioMan.sfx[g].num_types;i++)
      for(int s = 0; s< AudioMan.sfx[g].items[i].num_sounds;s++)
        SetSoundVolume(AudioMan.sfx[g].items[i].sounds[s],AudioMan.sfx[g].volume);

    StepEvents(AudioMan.timers[g]);
  }
/*
  int curTrack = AudioMan.current_track;
  int curSong = AudioMan.tracks[curTrack].current_index;
  if(AudioMan.tracks[curTrack].track[curSong].events)
    StepEvents(AudioMan.tracks[curTrack].track[curSong].events);

  float vol = AudioMan.tracks[curTrack].volume * AudioMan.tracks[curTrack].track[curSong].fade->vol; 
  UpdateMusicStream(AudioMan.tracks[curTrack].track[curSong].music);
  SetMusicVolume(AudioMan.tracks[curTrack].track[curSong].music,vol);
  if(AudioMan.concurrent_track != -1){ 
    curTrack = AudioMan.concurrent_track;
    curSong = AudioMan.tracks[curTrack].current_index;
  
    if(AudioMan.tracks[curTrack].track[curSong].events)
      StepEvents(AudioMan.tracks[curTrack].track[curSong].events);

    vol = AudioMan.tracks[curTrack].volume * AudioMan.tracks[curTrack].track[curSong].fade->vol; 
    UpdateMusicStream(AudioMan.tracks[curTrack].track[curSong].music);
    SetMusicVolume(AudioMan.tracks[curTrack].track[curSong].music,vol);

  }
*/
}
