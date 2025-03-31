/** Network dependent widgets. */
var vehicle_state;
var vehicle_temp;
var vehicle_volt;
var vehicle_cur;
var vehicle_soc;
var vehicle_req_cur;
var vehicle_status_flags;
var vehicle_error_flags;
var vehicle_error_line;
var vehicle_normal_term_line;

var charger_avail_volt;
var charger_avail_cur;
var charger_output_volt;
var charger_output_cur;
var charger_threshold_volt;
var charger_trem_10;
var charger_trem_60;
var charger_status;

params = new layout(widget.root, widget.horizontal);
	w = new wrapper(params);
	w._element.style.background = "white";
		t = new titled_text(w, "Vehicle");
		content = new layout(w, widget.horizontal);
			l = new layout(content, widget.vertical);
			l.width = "min";
				new text_box(l, "State");
				
				new gap(l);
				new text_box(l, "temperature");
				
				new gap(l);
				new text_box(l, "voltage");
				new text_box(l, "current");
				new text_box(l, "soc");
				
				new gap(l);
				new text_box(l, "req current");
				
				new gap(l);
				new text_box(l, "status flags");
				new text_box(l, "error flags");
				new text_box(l, "error line");
				new text_box(l, "nterm line");

			l = new layout(content, widget.vertical);
				vehicle_state = new text_box(l, "0");
				new gap(l);
				vehicle_temp = new text_box(l, "0");
				new gap(l);
				vehicle_volt = new text_box(l, "0");
				vehicle_cur = new text_box(l, "0");
				vehicle_soc = new text_box(l, "0");
				new gap(l);
				vehicle_req_cur = new text_box(l, "0");
				new gap(l);
				vehicle_status_flags = new text_box(l, "0");
				vehicle_error_flags = new text_box(l, "0");
				vehicle_error_line = new text_box(l, "0");
				vehicle_normal_term_line
					= new text_box(l, "0");

	w = new wrapper(params);
	w._element.style.background = "orange";
		t = new titled_text(w, "Charger");
		content = new layout(w, widget.horizontal);
			l = new layout(content, widget.vertical);
			l.width = "min";	
			
				new titled_text(l, "avail");
				new text_box(l, "voltage");
				new text_box(l, "current");

				new gap(l);
				new titled_text(l, "output");
				new text_box(l, "voltage");
				new text_box(l, "current");

				new gap(l);
				new titled_text(l, "threshold");
				new text_box(l, "voltage");

				new gap(l);
				new titled_text(l, "remaining");
				new text_box(l, "time by 10s");
				new text_box(l, "time by 60s");
				
				new gap(l);
				new text_box(l, "status");

			l = new layout(content, widget.vertical);
				t = new titled_text(l, "_");
				charger_avail_volt = new text_box(l, "0");
				charger_avail_cur = new text_box(l, "0");

				new gap(l);
				new titled_text(l, "_");
				charger_output_volt = new text_box(l, "0");
				charger_output_cur = new text_box(l, "0");

				new gap(l);
				new titled_text(l, "_");
				charger_threshold_volt = new text_box(l, "0");

				new gap(l);
				new titled_text(l, "_");
				charger_trem_10 = new text_box(l, "0");
				charger_trem_60 = new text_box(l, "0");
				
				new gap(l);
				charger_status = new text_box(l, "0");
params.id = 0;
params.on_recv = function(args) {
	vehicle_state.text = args[1];
	vehicle_temp.text = args[2];
	vehicle_volt.text = args[3];
	vehicle_cur.text = args[4];
	vehicle_soc.text = args[5];
	vehicle_req_cur.text = args[6];
	vehicle_status_flags.text = args[7];
	vehicle_error_flags.text = args[8];
	vehicle_error_line.text = args[9];
	vehicle_normal_term_line.text = args[10];

	charger_avail_volt.text = args[11];
	charger_avail_cur.text = args[12];
	charger_output_volt.text = args[13];
	charger_output_cur.text = args[14];
	charger_threshold_volt.text = args[15];
	charger_trem_10.text = args[16];
	charger_trem_60.text = args[17];
	charger_status.text = args[18];
}

new gap(widget.root);
estop = new button(widget.root, "Emergency stop")
estop.id = 1;

new gap(widget.root);
w = new wrapper(widget.root);
	upd = new updater(w, "Update firmware", "Update");

new gap(widget.root);
new text_box(widget.root, "Version 1.0");
