boolean PTlogopen = false;
boolean PT2logopen = false;

void openPTlog() {
  if (!PTlogopen) {
    ptLog = SD.open(filename, FILE_WRITE);
    PTlogopen = true;
  }
}

void closePTlog() {
  if (PTlogopen) {
    ptLog.close();
    PTlogopen = false;
  }
}

void openPT2log() {
  if (!PT2logopen) {
    pt2Log = SD.open(filename2, FILE_WRITE);
    PT2logopen = true;
  }
}

void closePT2log() {
  if (PT2logopen) {
    pt2Log.close();
    PT2logopen = false;
  }
}
