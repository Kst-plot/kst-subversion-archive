/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void KstFFTOptions::init()
{
    update();
}

void KstFFTOptions::update()
{
    KST::vectorDefaults.sync();

    FFTLen->setValue(KST::vectorDefaults.fftLen());
    SampRate->setText(QString::number(KST::vectorDefaults.psdFreq()));
    VectorUnits->setText(KST::vectorDefaults.vUnits());
    RateUnits->setText(KST::vectorDefaults.rUnits());
    Apodize->setChecked(KST::vectorDefaults.apodize());
    RemoveMean->setChecked(KST::vectorDefaults.removeMean());
    Interleaved->setChecked(KST::vectorDefaults.psdAverage());

    clickedInterleaved();
    clickedApodize();
    changedApodizeFxn();
}

void KstFFTOptions::changedApodizeFxn() {
    int gaussianIndex = 5;
    if (ApodizeFxn->text(0) == "") {
      gaussianIndex++;
    }
    Sigma->setEnabled(ApodizeFxn->currentItem() == gaussianIndex && Apodize->isChecked());
}

void KstFFTOptions::clickedInterleaved()
{
    FFTLen->setEnabled(Interleaved->isChecked());
}

void KstFFTOptions::clickedApodize()
{
    ApodizeFxn->setEnabled(Apodize->isChecked());
}

void KstFFTOptions::synch()
{
    clickedInterleaved();
    clickedApodize();
}

bool KstFFTOptions::checkValues()
{
  double new_freq = SampRate->text().toDouble();
  int new_len = FFTLen->text().toInt();
  return checkGivenValues(new_freq, new_len);
}

bool KstFFTOptions::checkGivenValues(double sampRate, int FFTLen)
{
  if (sampRate <= 0) {
    KMessageBox::sorry(this, i18n("The sample rate must be greater than 0"));
    return false;
  }
  if (FFTLen < 2) {
    KMessageBox::sorry(this, i18n("The FFT length must be greater than 2^2"));
    return false;
  }
  return true;
}

// vim: ts=8 sw=4 noet
