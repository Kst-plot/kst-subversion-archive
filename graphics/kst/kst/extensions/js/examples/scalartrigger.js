
function ScalarTrigger() {
	this.Watcher = function(name, comparator, value) {
		this.scalar = data.scalar(name);
		this.watch_void = function() {
			if (this.scalar && eval(Math.abs(this.scalar.value()).toString() + this.comp + this.value)) {
				if (!this.triggered) {
					alert("Triggered!  Value is: " + this.scalar.value());
					this.triggered = true;
				}
			} else {
				this.triggered = false;
			}
		};

		this.triggered = false;
		this.comp = comparator;
		this.value = value;

		if (this.scalar) {
			this.scalar.connect(this.scalar, 'trigger()', this, 'watch_void');
		}
	}


	this.watches = Array();

        this.newWatcher_void = function() {
		var dlg = this.dialog;
		var s = dlg.getElementById('scalars');
		if (s.currentItem == -1) {
			alert("You must select a scalar to apply the trigger to.");
			return;
		}

		var w = new this.Watcher(s.currentText, dlg.getElementById('comparator').currentText, dlg.getElementById('value').text);
		this.watches[this.watches.length] = w;
		s.clearSelection();
	};


	this.dialog = Factory.loadui("scalartriggerdialog.ui");
	var button = this.dialog.getElementById('ok');
	button.connect(button, 'clicked()', this, 'newWatcher_void');

	this.show = function() {
		var list = this.dialog.getElementById('scalars');
		list.clear();
		for (var i = 0; i < data.scalars().length; ++i) {
			list.insertItem(data.scalars()[i]);
		}
		this.dialog.show();
		this.dialog.raise();
	}

	this.hide = function() {
		this.dialog.hide();
	}

	var gui = KstUIMerge.loadGUI('scalartrigger.rc');
	this.action = new KAction(gui.actionCollection(), 'kst_scalar_trigger');
	this.action.text = "Scalar Triggers";
	this.action.connect(this.action, 'activated()', this, 'show');
	gui.merge();
}

KstScriptRegistry.addScript("Scalar Trigger", "ScalarTrigger");

