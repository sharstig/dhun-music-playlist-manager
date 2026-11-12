/**
 * audio.js - Web Audio API Synthesizer & Playback Engine
 * Provides actual, 100% offline, soothing synth playback tailored to each song's theme.
 */

class DhunAudioEngine {
  constructor() {
    this.audioCtx = null;
    this.isPlaying = false;
    this.currentSong = null;
    this.currentTime = 0;
    this.duration = 180;
    this.timerInterval = null;
    this.volume = 0.8;
    this.isMuted = false;

    // Synth nodes
    this.masterGain = null;
    this.analyser = null;
    this.activeOscillators = [];
    this.onProgressCallback = null;
    this.onEndCallback = null;
    this.onStateChangeCallback = null;

    // Presets with note frequencies (Hz) for chords and bass
    this.presets = {
      sunset: {
        chords: [[261.63, 329.63, 392.00, 493.88], [220.00, 261.63, 329.63, 392.00], [174.61, 220.00, 261.63, 329.63], [196.00, 246.94, 293.66, 349.23]],
        bass: [130.81, 110.00, 87.31, 98.00],
        type: 'sine',
        tempo: 2.5
      },
      neon: {
        chords: [[293.66, 349.23, 440.00], [233.08, 293.66, 349.23], [174.61, 220.00, 261.63], [130.81, 164.81, 196.00]],
        bass: [73.42, 58.27, 87.31, 65.41],
        type: 'sawtooth',
        tempo: 1.8
      },
      monsoon: {
        chords: [[220.00, 277.18, 329.63, 415.30], [246.94, 311.13, 369.99], [220.00, 261.63, 329.63], [196.00, 246.94, 293.66]],
        bass: [110.00, 123.47, 110.00, 98.00],
        type: 'triangle',
        tempo: 3.0
      },
      aurora: {
        chords: [[329.63, 392.00, 493.88, 587.33], [293.66, 369.99, 440.00, 523.25], [261.63, 329.63, 392.00, 493.88]],
        bass: [82.41, 73.42, 65.41],
        type: 'sine',
        tempo: 3.5
      },
      coffee: {
        chords: [[261.63, 329.63, 392.00, 466.16], [293.66, 349.23, 440.00, 523.25], [196.00, 246.94, 293.66, 349.23]],
        bass: [130.81, 146.83, 98.00],
        type: 'triangle',
        tempo: 2.2
      },
      galaxy: {
        chords: [[146.83, 220.00, 293.66, 369.99], [130.81, 196.00, 261.63, 329.63], [110.00, 164.81, 220.00, 277.18]],
        bass: [55.00, 49.00, 41.20],
        type: 'sine',
        tempo: 4.0
      }
    };
  }

  initAudioContext() {
    if (!this.audioCtx) {
      const AudioCtxClass = window.AudioContext || window.webkitAudioContext;
      this.audioCtx = new AudioCtxClass();
      this.masterGain = this.audioCtx.createGain();
      this.analyser = this.audioCtx.createAnalyser();
      this.analyser.fftSize = 64;

      this.masterGain.gain.setValueAtTime(this.volume, this.audioCtx.currentTime);
      this.masterGain.connect(this.analyser);
      this.analyser.connect(this.audioCtx.destination);
    }

    if (this.audioCtx.state === 'suspended') {
      this.audioCtx.resume();
    }
  }

  loadSong(song) {
    this.currentSong = song;
    this.currentTime = 0;
    this.duration = song.duration || 180;
    this.stopSynth();
    if (this.onProgressCallback) {
      this.onProgressCallback(this.currentTime, this.duration);
    }
  }

  play() {
    this.initAudioContext();
    if (!this.currentSong) return;

    this.isPlaying = true;
    this.startSynth();
    this.startProgressTimer();

    if (this.onStateChangeCallback) {
      this.onStateChangeCallback(true);
    }
  }

  pause() {
    this.isPlaying = false;
    this.stopSynth();
    this.stopProgressTimer();

    if (this.onStateChangeCallback) {
      this.onStateChangeCallback(false);
    }
  }

  togglePlay() {
    if (this.isPlaying) {
      this.pause();
    } else {
      this.play();
    }
  }

  startSynth() {
    this.stopSynth();
    if (!this.audioCtx || !this.currentSong) return;

    const themeKey = this.currentSong.coverUrl || 'sunset';
    const preset = this.presets[themeKey] || this.presets.sunset;

    try {
      // Create Ambient Pad chord oscillators
      const chordIndex = Math.floor((this.currentTime / preset.tempo) % preset.chords.length);
      const notes = preset.chords[chordIndex];

      notes.forEach((freq, idx) => {
        const osc = this.audioCtx.createOscillator();
        const gain = this.audioCtx.createGain();

        osc.type = preset.type;
        osc.frequency.setValueAtTime(freq, this.audioCtx.currentTime);

        // Soft attack
        gain.gain.setValueAtTime(0.001, this.audioCtx.currentTime);
        gain.gain.exponentialRampToValueAtTime(0.08 / notes.length, this.audioCtx.currentTime + 0.8);

        osc.connect(gain);
        gain.connect(this.masterGain);
        osc.start();

        this.activeOscillators.push({ osc, gain });
      });

      // Gentle sub-bass pulse
      const bassFreq = preset.bass[chordIndex % preset.bass.length];
      const bassOsc = this.audioCtx.createOscillator();
      const bassGain = this.audioCtx.createGain();

      bassOsc.type = 'sine';
      bassOsc.frequency.setValueAtTime(bassFreq, this.audioCtx.currentTime);

      bassGain.gain.setValueAtTime(0.001, this.audioCtx.currentTime);
      bassGain.gain.exponentialRampToValueAtTime(0.12, this.audioCtx.currentTime + 0.4);

      bassOsc.connect(bassGain);
      bassGain.connect(this.masterGain);
      bassOsc.start();

      this.activeOscillators.push({ osc: bassOsc, gain: bassGain });
    } catch (e) {
      console.warn('[AUDIO] Synth generation notice:', e);
    }
  }

  stopSynth() {
    if (!this.activeOscillators) return;
    this.activeOscillators.forEach(({ osc, gain }) => {
      try {
        if (this.audioCtx && gain) {
          gain.gain.setValueAtTime(gain.gain.value, this.audioCtx.currentTime);
          gain.gain.exponentialRampToValueAtTime(0.0001, this.audioCtx.currentTime + 0.1);
        }
        setTimeout(() => {
          try { osc.stop(); osc.disconnect(); } catch (_) {}
        }, 120);
      } catch (_) {}
    });
    this.activeOscillators = [];
  }

  startProgressTimer() {
    this.stopProgressTimer();
    this.timerInterval = setInterval(() => {
      if (!this.isPlaying) return;

      this.currentTime += 1;

      // Chord progression shift every few seconds
      const themeKey = this.currentSong ? this.currentSong.coverUrl : 'sunset';
      const preset = this.presets[themeKey] || this.presets.sunset;
      if (Math.floor(this.currentTime) % Math.round(preset.tempo) === 0) {
        this.startSynth();
      }

      if (this.onProgressCallback) {
        this.onProgressCallback(this.currentTime, this.duration);
      }

      // Track completed -> trigger circular advance to next song!
      if (this.currentTime >= this.duration) {
        this.pause();
        if (this.onEndCallback) {
          this.onEndCallback();
        }
      }
    }, 1000);
  }

  stopProgressTimer() {
    if (this.timerInterval) {
      clearInterval(this.timerInterval);
      this.timerInterval = null;
    }
  }

  seek(timeInSeconds) {
    this.currentTime = Math.max(0, Math.min(timeInSeconds, this.duration));
    if (this.onProgressCallback) {
      this.onProgressCallback(this.currentTime, this.duration);
    }
    if (this.isPlaying) {
      this.startSynth();
    }
  }

  setVolume(val) {
    this.volume = Math.max(0, Math.min(1, val));
    if (this.masterGain && this.audioCtx) {
      this.masterGain.gain.setValueAtTime(this.isMuted ? 0 : this.volume, this.audioCtx.currentTime);
    }
  }

  toggleMute() {
    this.isMuted = !this.isMuted;
    if (this.masterGain && this.audioCtx) {
      this.masterGain.gain.setValueAtTime(this.isMuted ? 0 : this.volume, this.audioCtx.currentTime);
    }
    return this.isMuted;
  }
}

window.DhunAudioEngine = DhunAudioEngine;
