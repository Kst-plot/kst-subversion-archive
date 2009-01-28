// plotfiles() keeps prompting for filenames
// and graphs the contents and their psds until
// you click cancel.

function plotfiles() {
  wdat = Kst.windows['data'];
  if (!wdat) { wdat = new Window('data'); }

  wpsd = Kst.windows['psds'];
  if (!wpsd) { wpsd = new Window('psds'); }

  while (filename = prompt("Enter a filename...") ) {
    tsrc = new DataSource(filename);
    tsrc.tagName = filename;

    tx = new DataVector(tsrc, '1');
    tx.tagName = tsrc.tagName + '-x';
    ty = new DataVector(tsrc, '2');
    ty.tagName = tsrc.tagName + '-y';

    tpd = new Plot(wdat);
    tpd.tagName = tsrc.tagName + '-plot';

    tdc = new Curve(tx,ty);
    tdc.tagName = tsrc.tagName + '-C';
    tpd.curves.append(tdc);

    tps = new PowerSpectrum(ty,1);
    tps.tagName = ty.tagName + "-PS";

    tpps = new Plot(wpsd);
    tpps.tagName = tps.tagName + '-plot';

    tpsc = new Curve(tps.xVector, tps.yVector);
    tpsc.tagName = tps.tagName + '-PSC';
    tpps.curves.append(tpsc);
  }
}
