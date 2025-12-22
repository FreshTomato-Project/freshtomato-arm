function getNextpage() {
	var f = (window.fields && fields.getAll) ? fields.getAll(document.body) : [];
	for (var i = 0; i < f.length; ++i)
		if (f[i].name === "_nextpage" && f[i].value)
			return f[i].value;
	var a = asp();
	return a ? a.replace(/[\.\-]/g, "_") : "";
}

function getModelVersion() {
	var m = nvram.t_model_name || "unknown"; 
    var v = nvram.os_version;
    var versionMatch = v.match(/\d{4}\.\d+/);
    v = versionMatch ? versionMatch[0] : v;
    return {
        model: m.replace(/[^a-zA-Z0-9_\-\.]+/g, "_"),
        version: v.replace(/[^a-zA-Z0-9_\-\.]+/g, "_")
    };
}

function backupGrid() {
	var np = getNextpage();
	if (!np) { alert("Unable to detect page name (_nextpage or filename)"); return; }
	var b = {nextpage:np,grids:{}};
	var t = document.getElementsByTagName('table');
	for (var i = 0; i < t.length; ++i) {
		var tbl = t[i], g = tbl.gridObj;
		if (g && g.getAllData) {
			var id = tbl.id || ("grid" + i),
				rows = g.getAllData(), cols = [];
			if (g.columns) for (var j = 0; j < g.columns.length; ++j)
				if (!g.columns[j].readonly && g.columns[j].edit !== false) cols.push(j);
			b.grids[id] = rows.map(function(r) {
				if (!Array.isArray(r) || !cols.length) return r;
				for (var d=[],k=0; k<cols.length; ++k) d.push(r[cols[k]]);
				return d;
			});
		}
	}
	var meta = getModelVersion(),
		pg = np.replace(/[^a-zA-Z0-9_-]+/g, "_") || "unknown",
		d = new Date(),
		ts = d.getFullYear()+"-"+(("0"+(d.getMonth()+1)).slice(-2))+"-"+(("0"+d.getDate()).slice(-2))+"_"+(("0"+d.getHours()).slice(-2))+(("0"+d.getMinutes()).slice(-2)),
		fn = meta.model+"-"+meta.version+"-"+pg+"-backup-"+ts+".json",
		j = JSON.stringify(b, null, 2),
		a = document.createElement('a');
	a.href = 'data:application/json;charset=utf-8,' + encodeURIComponent(j);
	a.download = fn;
	a.style.display = 'none';
	document.body.appendChild(a);
	a.click();
	document.body.removeChild(a);
}

function restoreGrid(j) {
	var b;
	try { b = (typeof j == "string") ? JSON.parse(j) : j; }
	catch(e) { alert("Invalid backup JSON."); return; }
	var currNext = getNextpage();
	if (!currNext) { alert("Cannot restore: missing _nextpage field or filename."); return; }
	if (b.nextpage !== currNext) {
		alert("Restore denied!\nThis backup is for '"+(b.nextpage||"unknown")+"',\nbut this page is '"+currNext+"'.");
		return;
	}
	var gr = 0, gs = 0, warnings = [];
	if (b.grids) {
		var t = document.getElementsByTagName('table');
		for (var i = 0; i < t.length; ++i) {
			var tbl = t[i], g = tbl.gridObj, id = tbl.id || ("grid"+i);
			if (g && g.insertData && Array.isArray(b.grids[id])) {
				try {
					var enableBox = null;
					var p = tbl;
					for (var lvl=0; lvl<2 && p; ++lvl) {
						var ebs = p.querySelectorAll && p.querySelectorAll('input[type=checkbox][name*="enable"]');
						if (ebs && ebs.length) { enableBox = ebs[0]; break; }
						p = p.parentNode;
					}
					if (enableBox && enableBox.disabled) {
						enableBox.disabled = false;
						enableBox.checked = true;
						warnings.push("Enabled a section to allow grid restore (checkbox '"+enableBox.name+"').");
					}
					if (g.removeAllData) g.removeAllData();
					b.grids[id].forEach(function(r){ g.insertData(-1, r); });
					gr++;
					if (typeof g.getAllData === "function" && g.getAllData().length > b.grids[id].length) {
						var nrm = g.getAllData().length-b.grids[id].length;
						for (var rm=0; rm<nrm; ++rm) if (g.removeRow) g.removeRow(g.getAllData().length-1);
					}
				} catch(e) { gs++; }
			}
		}
	}
	var msg = "";
	if (gr) msg += "Successfully restored table.\n";
	if (gs) msg += "Failed to restore table(s).\n";
    if (!gr && !gs) msg += "No grid data found to restore.\n";
	if (warnings.length) msg += "\n\nWarnings:\n- " + warnings.join("\n- ");
	alert(msg);
}

function restore() {
    var fi = document.createElement('input');
    fi.type = 'file';
    fi.accept = '.json,application/json';
    fi.style.display = 'none';
    document.body.appendChild(fi); 
    fi.onchange = function(e){
		var f = e.target.files[0];
		if (!f) return;
		var r = new FileReader();
		r.onerror = function(){ alert("Failed to read backup file."); };
		r.onload = function(x){ try { restoreGrid(x.target.result); } catch(ex) { alert("Restore failed: "+(ex.message||ex)); } };
		r.readAsText(f);
		fi.value = "";
	};
  fi.click();
}

function clearGrid() {
    if (!confirm("Are you sure you want to remove ALL entries from this table?\n\nThis will not be permanent until you click the 'Save' button at the bottom.")) {
        return;
    }
    var grid = null;
    var container = E('sm-grid') || E('bs-grid') || E('fo-grid') || E('fo-grid6');
    if (container && container.gridObj) {
        grid = container.gridObj;
    } else {
        var allGrids = document.querySelectorAll('.tomato-grid');
        for (var i = 0; i < allGrids.length; i++) {
            if (allGrids[i].gridObj) {
                grid = allGrids[i].gridObj;
                break;
            }
        }
    }
    if (!grid) grid = (window.wfgrid || window.sg || window.tg);
    if (grid && grid.removeAllData) {
        grid.removeAllData();
        grid.recolor();
        if (grid.update) grid.update();
    } else {
        alert("Error: Table object not found. Make sure gridObj is linked in earlyInit.");
    }
}
