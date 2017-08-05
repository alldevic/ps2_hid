#pragma once

#include <Arduino.h>

class KeyReport {
  public:
  KeyReport* KeyReport::operator &(){ return this; }
  void add(uint8_t k) {
    uint8_t i;
    if (k >= 224) this->modifiers_ |= 1 << (k - 224);
    else if (this->keys_[0] != k && this->keys_[1] != k &&
             this->keys_[2] != k && this->keys_[3] != k &&
             this->keys_[4] != k && this->keys_[5] != k) {
      for (i = 0; i < 6; ++i)
        if (this->keys_[i] == 0) {
          this->keys_[i] = k;
          break;
        }
    }
  }

  void remove(uint8_t k) {
    uint8_t i;
    if (k >= 224) this->modifiers_ &= ~(1 << (k - 224));
    else {
      for (i = 0; i < 6; ++i) {
        if (this->keys_[i] == k) {
          this->keys_[i] = 0;
          break;
        }
      }
    }
  }

  uint8_t get_modifiers() {return this->modifiers_; }
  
  uint8_t get_key(uint8_t key) {return this->keys_[key];}
  private:
  uint8_t modifiers_;
  uint8_t reserved_;
  uint8_t keys_[6];
};


