
function QuietRiot() {
		for (w = 0; w < Kst.windows.length; ++w) {
			win = Kst.windows[w];
			for (p = 0; p < win.plots.length; ++p) {
				for (i = 0; i < 360; ++i) {
					plot = win.plots[p];
					plot.xAxis.tickLabel.rotation = (plot.xAxis.tickLabel.rotation + 1) % 360;
					plot.yAxis.tickLabel.rotation = (plot.yAxis.tickLabel.rotation + 1) % 360;
				}
			}
		}
}

KstScriptRegistry.addScript("Quiet Riot", "QuietRiot");

