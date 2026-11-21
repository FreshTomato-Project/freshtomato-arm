<?php
/**
 * AdminNeo - Powerful database manager in a single PHP file
 * v5.9.1
 *
 * Compiled with
 * drivers:   mysql
 * languages: en
 * themes:    default-blue
 * config:    no
 *
 * @link https://www.adminneo.org/
 *
 * @author Peter Knut
 * @author Jakub Vrana (https://www.vrana.cz/)
 *
 * @copyright 2007-2025 Jakub Vrána
 * @copyright 2024-2025 Peter Knut
 *
 * @license Apache License, Version 2.0 (https://www.apache.org/licenses/LICENSE-2.0)
 * @license GNU General Public License, version 2 (https://www.gnu.org/licenses/gpl-2.0.html)
 */namespace
AdminNeo;use
Exception;use
stdClass;use
PDO;use
PDOStatement;use
mysqli;use
mysqli_result;abstract
class
Plugin{protected$admin;protected$config;protected$settings;protected$locale;function
inject($ya,Config$Wb,Settings$P,Locale$Ug){$this->admin=$ya;$this->config=$Wb;$this->settings=$P;$this->locale=$Ug;}}abstract
class
Origin
extends
Plugin{private$errors=[];private
static$instance=null;static
function
create(array$Wb=[],array$pj=[]){if(self::$instance)die("Admin instance already exists.\n");$ya=new
static();if(!$Wb&&file_exists("adminneo-config.php")){$Wb=include_once("adminneo-config.php");if(!is_array($Wb)){$Wb=[];$Lg="href=https://github.com/adminneo-org/adminneo#configuration ".target_blank();$ya->addError(lang(0,"<b>adminneo-config.php</b>")." <a $Lg>".lang(1)."</a>");}}$Wb=new
Config($Wb);$P=new
Settings($Wb);if(!$pj&&file_exists("adminneo-plugins.php")){$pj=include_once("adminneo-plugins.php");if(!is_array($pj)){$pj=[];$Lg="href=https://github.com/adminneo-org/adminneo#plugins ".target_blank();$ya->addError(lang(0,"<b>adminneo-plugins.php</b>")." <a $Lg>".lang(1)."</a>");}}self::$instance=$pj?new
Pluginer($ya,$pj):$ya;$ya->inject(self::$instance,$Wb,$P,Locale::get());foreach($pj
as$oj)$oj->inject(self::$instance,$Wb,$P,Locale::get());return
self::$instance;}static
function
get(){if(!self::$instance)die("Admin instance not found. Create instance by Admin::create() method at first.\n");return
self::$instance;}protected
function
__construct(){}function
getConfig(){return$this->config;}function
getSettings(){return$this->settings;}abstract
function
getOperators();function
getLikeOperator(){return
Driver::get()->getLikeOperator();}function
getRegexpOperator(){return
null;}function
init(){}function
addError($j){$this->errors[]=$j;}function
getErrors(){return$this->errors;}abstract
function
getServiceTitle();function
getCredentials(){$O=$this->config->getServer(SERVER);return[$O?$O->getServer():SERVER,$_GET["username"],get_password()];}function
verifyDefaultPassword($F){$Xe=$this->config->getDefaultPasswordHash();if($Xe===null||$Xe==="")return
lang(2);elseif(!password_verify($F,$Xe))return
lang(3);return
true;}function
authenticate($V,$F){if($F==""){$Xe=$this->config->getDefaultPasswordHash();if($Xe===null)return
lang(4,target_blank());else
return$Xe==="";}return
true;}function
getPrivateKey($e=false){return
get_private_key($e);}function
getBruteForceKey(){return$_SERVER["REMOTE_ADDR"];}function
getServerName($O,$gk=true,$Wd=null){if($O==""){if(!$gk)return"";$O=Connection::exists()?Connection::get()->getDefaultServerName():"";if($O=="")return$Wd!==null?$Wd:lang(5);$Qk=null;}else$Qk=$this->config->getServer($O);return$Qk?$Qk->getName():preg_replace('~^https?://~',"",$O);}abstract
function
getDatabase();function
getDatabases($pe=true){$g=$this->filterListWithWildcards(get_databases($pe),$this->config->getHiddenDatabases(),false,Driver::get()->getSystemDatabases());if(DB!=""&&!in_array(DB,$g))array_unshift($g,DB);return$g;}function
getSchemas($Vh=false){$df=$this->config->getHiddenSchemas();if($Vh&&!in_array("__system",$df))$df[]="__system";$Ak=$this->filterListWithWildcards(schemas(),$df,false,Driver::get()->getSystemSchemas());if(isset($_GET["ns"])&&$_GET["ns"]!=""&&!in_array($_GET["ns"],$Ak))array_unshift($Ak,$_GET["ns"]);return$Ak;}function
getCollations(array$ig=[]){$wn=$this->config->getVisibleCollations();$je=$wn?array_merge($wn,$ig):[];return$this->filterListWithWildcards(collations(),$je,true);}private
function
filterListWithWildcards(array$nn,array$je,$kg,array$Nl=[]){if(!$nn||!$je)return$nn;$s=array_search("__system",$je);if($s!==false){unset($je[$s]);$je=array_merge($je,$Nl);}array_walk($je,function(&$Y){$Y=str_replace('\\*',".*",preg_quote($Y,"~"));});$ij='~^('.implode("|",$je).')$~';return$this->filterListWithPattern($nn,$ij,$kg);}private
function
filterListWithPattern(array$nn,$ij,$kg){$J=[];foreach($nn
as$u=>$Y){if(is_array($Y)){if($Dl=$this->filterListWithPattern($Y,$ij,$kg))$J[$u]=$Dl;}elseif(($kg&&preg_match($ij,$Y))||(!$kg&&!preg_match($ij,$Y)))$J[$u]=$Y;}return$J;}abstract
function
getQueryTimeout();function
sendHeaders(){}function
updateCspHeader(array&$nc){}function
printFavicons(){$Gb=validate_color_variant($this->config->getColorVariant());echo"<link rel='icon' type='image/x-icon' href='",h(link_files("favicon-$Gb.ico",[])),"' sizes='32x32'>\n","<link rel='icon' type='image/svg+xml' href='",h(link_files("favicon-$Gb.svg",[])),"'>\n","<link rel='apple-touch-icon' href='",h(link_files("apple-touch-icon-$Gb.png",[])),"'>\n";}abstract
function
printToHead();function
getCssUrls(){$bn=$this->config->getCssUrls();foreach(["adminneo.css","adminneo-light.css","adminneo-dark.css"]as$n){if(file_exists($n))$bn[]="$n?v=".filemtime($n);}return$bn;}function
isLightModeForced(){return$this->isColorSchemeForced(false);}function
isDarkModeForced(){return$this->isColorSchemeForced(true);}private
function
isColorSchemeForced($sc){$zh=$sc?Settings::$ColorSchemeDark:Settings::$ColorSchemeLight;$_h=$sc?Settings::$ColorSchemeLight:Settings::$ColorSchemeDark;$fe=file_exists("adminneo-$zh.css");$ge=file_exists("adminneo-$_h.css");if($fe&&!$ge)return
true;return$this->settings->getColorScheme()==$zh&&!($fe
xor$ge);}function
getJsUrls(){$bn=$this->config->getJsUrls();$n="adminneo.js";if(file_exists($n))$bn[]="$n?v=".filemtime($n);return$bn;}abstract
function
printLoginForm();function
getLoginFormRow($ae,$sg,$k){if($sg)return"<tr><th>$sg</th><td>$k</td></tr>\n";else
return"$k\n";}function
printLogout(){echo"<div class='logout'>","<form action='' method='post'>\n","<span title='",lang(6),"'>",h($_GET["username"]),"</span>","<input type='submit' class='button' name='logout' value='",lang(7),"' id='logout'>",input_token(),"</form>","</div>\n";}function
getTableName(array$Rl){return
h($Rl["Name"]);}abstract
function
getFieldName(array$k,$D=0);function
formatComment($Ob){return
h($Ob);}abstract
function
printTableMenu(array$Rl,$Gf);function
getForeignKeys($Q){return
foreign_keys($Q);}function
getBackwardKeys($Q,$Pl){if(!$this->settings->isRelationLinks())return[];$M=backward_keys($Q);$mg=[];foreach($M
as$L){$r=$L["table_schema"].".".$L["table_name"];$mg[$r]["schema"]=$L["table_schema"];$mg[$r]["table"]=$L["table_name"];$mg[$r]["constraints"][$L["constraint_name"]][$L["column_name"]]=$L["referenced_column_name"];}foreach($mg
as$r=>$u){$A=$this->admin->getTableName(table_status1($u["table"],true));if($A!=""){$Ck=preg_quote($Pl);$Nk="(:|\\s*-)?\\s+";$mg[$r]["name"]=(preg_match("(^$Ck$Nk(.+)|^(.+?)$Nk$Ck\$)iu",$A,$z)?$z[2].$z[3]:$A);}else
unset($mg[$r]);}return$mg;}function
printBackwardKeys(array$Wa,array$L){foreach($Wa
as$u){foreach($u["constraints"]as$Zb){$kh=preg_replace('~&ns=[^&]+&~',"&ns=".urldecode($u["schema"])."&",ME);$x=$kh.'select='.urlencode($u["table"]);$q=0;foreach($Zb
as$b=>$X){if(!isset($L[$X]))continue
2;$x
.=where_link($q++,$b,$L[$X]);}$A=preg_replace('(^'.preg_quote($_GET["select"]).(substr($_GET["select"],-1)=="s"?"?":"").'_)',"_",$u["name"]);$T=implode(", ",array_keys($Zb));echo"<a href='".h($x)."' title='".h($T)."'>".h($A)."</a>";$x=$kh.'edit='.urlencode($u["table"]);foreach($Zb
as$b=>$X)$x
.="&preset".urlencode("[".bracket_escape($b)."]")."=".urlencode($L[$X]);echo"<a href='".h($x)."' title='".lang(8)."'>",icon_solo("add"),"</a> ";}}}abstract
function
formatSelectQuery($I,$tl,$Vd=false);abstract
function
formatMessageQuery($I,$qm,$Vd=false);abstract
function
formatSqlCommandQuery($I);function
printAfterSqlCommand(){}abstract
function
getTableDescriptionFieldName($Q);abstract
function
fillForeignDescriptions(array$M,array$se);function
getFieldValueLink($X,$k){if(is_mail($X))return"mailto:$X";if(is_web_url($X))return$X;return
null;}abstract
function
formatSelectionValue($X,$x,$k,$Li);abstract
function
formatFieldValue($Y,array$k);abstract
function
printTableStructure(array$l);abstract
function
printTablePartitions(array$Yi);abstract
function
printRelatedTables(array$S);abstract
function
printTableIndexes(array$t,array$Rl);abstract
function
printSelectionColumns(array$N,array$c);abstract
function
printSelectionSearch(array$Z,array$c,array$t);abstract
function
printSelectionOrder(array$D,array$c,array$t);abstract
function
printSelectionLimit($w);abstract
function
printSelectionLength($lm);abstract
function
printSelectionAction(array$t);function
isDataEditAllowed(){return!information_schema(DB);}abstract
function
processSelectionColumns(array$c,array$t);abstract
function
processSelectionSearch(array$l,array$t);abstract
function
processSelectionOrder(array$l,array$t);function
processSelectionLimit(){if(!isset($_GET["limit"]))return$this->settings->getRecordsPerPage();return$_GET["limit"]!=""?(int)$_GET["limit"]:0;}abstract
function
processSelectionLength();abstract
function
getFieldFunctions(array$k);abstract
function
getFieldInput($Q,array$k,$Na,$Y,$p);function
getFieldInputHint($Q,array$k,$Y){return
support("comment")?$this->admin->formatComment($k["comment"]):"";}abstract
function
processFieldInput(array$k,$Y,$p="");function
detectJson($be,&$Y,$_j=null){if(is_array($Y)){$ne=JSON_UNESCAPED_UNICODE|JSON_UNESCAPED_SLASHES|($this->config->isJsonValuesAutoFormat()?JSON_PRETTY_PRINT:0);$Y=json_encode($Y,$ne);return
true;}$ne=JSON_UNESCAPED_UNICODE|JSON_UNESCAPED_SLASHES|($_j?JSON_PRETTY_PRINT:0);if(preg_match('~^jsonb?$~',$be)){if($Y!=null&&$_j!==null&&$this->config->isJsonValuesAutoFormat()&&($gg=json_decode($Y))!==null)$Y=json_encode($gg,$ne);return
true;}if(!$this->config->isJsonValuesDetection())return
false;if(is_string($Y)&&$Y!=""&&preg_match('~varchar|text|character varying|String|keyword~',$be)&&($Y[0]=="{"||$Y[0]=="[")&&($gg=json_decode($Y))){if($_j!==null&&$this->config->isJsonValuesAutoFormat())$Y=json_encode($gg,$ne);return
true;}return
false;}function
getServerVariables(){return
show_variables();}function
getStatusVariables(){return
show_status();}function
getDumpFormats(){return['csv'=>['name'=>'CSV,','data'=>true],'csv;'=>['name'=>'CSV;','data'=>true],'tsv'=>['name'=>'TSV','data'=>true],];}function
getStructureDumpFormats(){return$this->filterDumpFormats("structure");}function
getDataDumpFormats(){return$this->filterDumpFormats("data");}private
function
filterDumpFormats($U){$we=[];foreach(self::get()->getDumpFormats()as$ve=>$Fc){if(is_string($Fc))$we[$ve]=$Fc;elseif(isset($Fc[$U])?$Fc[$U]:false)$we[$ve]=isset($Fc["name"])?$Fc["name"]:$ve;}return$we;}abstract
function
sendDumpFormatHeaders($of,$Dh=false);abstract
function
getDumpOutputs();abstract
function
sendDumpOutputHeaders($of,$Rd);function
sendDumpHeaders($of,$Dh=false){return
null;}function
dumpDatabase($wc){}abstract
function
dumpTable($Q,$Cl,$tn=0);abstract
function
dumpData($Q,$Cl,$I);abstract
function
getImportFilePath();abstract
function
printDatabaseMenu();abstract
function
printNavigation($xh);abstract
function
printDatabaseSwitcher($xh);function
printTablesFilter(){echo"<div class='tables-filter jsonly'>"."<input id='tables-filter' type='search' class='input' autocomplete='off' placeholder='".lang(9)."'>".script("initTablesFilter(".json_encode($this->admin->getDatabase(),JSON_HEX_TAG).");")."</div>\n";}abstract
function
printTableList(array$S);function
getSettingsRows($Pe){$P=[];if($Pe==1){$C=get_language_options();if($C)$P["lang"]="<tr><th id='label-language'>".lang(10)."</th>"."<td>".html_select("lang",get_language_options(),Locale::get()->getLanguage(),"","label-language")."</td></tr>\n";$vm=get_theme_titles($this->config->getColorVariant());if(count($vm)>1){list($nm)=validate_theme($this->config->getTheme(),$this->config->getColorVariant());$C=[""=>lang(11)." ($vm[$nm])"]+$vm;$P["theme"]="<tr><th id='label-theme'>".lang(12)."</th>"."<td>".html_select("theme",$C,($ra=$this->settings->getParameter("theme"))!==null?$ra:"","","label-theme")."</td></tr>\n";}$C=[""=>lang(13),Settings::$ColorSchemeLight=>lang(14),Settings::$ColorSchemeDark=>lang(15)];$P["colorScheme"]="<tr><th>".lang(16)."</th>"."<td>".html_radios("colorScheme",$C,($ra=$this->settings->getParameter("colorScheme"))!==null?$ra:"")."</td></tr>\n";}elseif($Pe==2){$C=[""=>lang(11),true=>lang(17),false=>lang(18),];$i=$C[$this->config->isRelationLinks()];$C[""].=" ($i)";$P["relationLinks"]="<tr><th>".lang(19)."</th>"."<td>".html_radios("relationLinks",$C,($ra=$this->settings->getParameter("relationLinks"))!==null?$ra:"")."<span class='input-hint'>".lang(20)."</span>"."</td></tr>\n";$i=$this->config->getRecordsPerPage();$C=[""=>lang(11)." ($i)","20","30","50","70","100",];$P["recordsPerPage"]="<tr><th id='label-records'>".lang(21)."</th>"."<td>".html_select("recordsPerPage",$C,($ra=$this->settings->getParameter("recordsPerPage"))!==null?$ra:"","","label-records")."<span class='input-hint'>".lang(22)."</span>"."</td></tr>\n";$i=($ra=$this->config->getEnumAsSelectThreshold())!==null?$ra:lang(23);$C=[""=>lang(11)." ($i)",-1=>lang(23),0=>lang(24),3=>lang(25,3),5=>lang(25,5),10=>lang(25,10),20=>lang(25,20),];$P["enumAsSelectThreshold"]="<tr><th id='label-enum'>".lang(26)."</th>"."<td>".html_select("enumAsSelectThreshold",$C,($ra=$this->settings->getParameter("enumAsSelectThreshold"))!==null?$ra:"","","label-enum",true)."<span class='input-hint'>".lang(27)."</span>"."</td></tr>\n";}return$P;}abstract
function
getForeignColumnInfo(array$se,$b);}class
Pluginer{private
static$InternalMethods=["inject"=>true,"getConfig"=>true,];private
static$AppendMethods=["getErrors"=>true,"getFieldFunctions"=>true,"getDumpOutputs"=>true,"getDumpFormats"=>true,"getSettingsRows"=>true,];private$plugins;private$hooks=[];function
__construct(Origin$ya,array$pj){$this->plugins=$pj;foreach(get_class_methods('\AdminNeo\Origin')as$vh){$this->hooks[$vh]=[];if(!(isset(self::$InternalMethods[$vh])?self::$InternalMethods[$vh]:false)){foreach($pj
as$oj){if(method_exists($oj,$vh))$this->hooks[$vh][]=$oj;}}if(isset(self::$AppendMethods[$vh])?self::$AppendMethods[$vh]:false)array_unshift($this->hooks[$vh],$ya);else$this->hooks[$vh][]=$ya;}}function
getPlugins(){return$this->plugins;}function
__call($A,array$Ti){$Ia=isset(self::$AppendMethods[$A])?self::$AppendMethods[$A]:false;$J=$Ia?[]:null;assert(isset($this->hooks[$A]),"Calling unknown plugin method: $A");foreach($this->hooks[$A]as$oj){$Y=call_user_func_array([$oj,$A],$Ti);if($Y!==null){if($Ia)$J+=$Y;else
return$Y;}}return$J;}function
updateCspHeader(array&$nc){$this->__call(__FUNCTION__,[&$nc]);}function
detectJson($be,&$Y,$_j=null){return$this->__call(__FUNCTION__,[$be,&$Y,$_j]);}}class
Admin
extends
Origin{function
getOperators(){return
Driver::get()->getOperators();}function
getServiceTitle(){return"<a href='".h(HOME_URL)."'><svg role='img' class='logo' width='133' height='28'><desc>AdminNeo</desc>"."<use href='".h(link_files("logo.svg",[]))."#logo'/></svg></a>";}function
getDatabase(){return
DB;}function
getQueryTimeout(){return
2;}function
printToHead(){echo"<link rel='stylesheet' href='",h(link_files("jush.css",[])),"'>";if(!$this->admin->isLightModeForced())echo"<link rel='stylesheet' ".(!$this->admin->isDarkModeForced()?"media='(prefers-color-scheme: dark)' ":"")."href='",h(link_files("jush-dark.css",[])),"'>\n";echo
script_src(link_files("jush.js",[]),true);}function
printLoginForm(){$ed=Drivers::getList();$Rk=$this->config->getServerPairs($ed);$O=SERVER?:$this->config->getDefaultServer();echo"<table class='box box-light'>\n";if($Rk)echo$this->admin->getLoginFormRow('server',lang(5),"<select name='auth[server]'>".optionlist($Rk,$O,true)."</select>");else{$cd=DRIVER?:$this->config->getDefaultDriver($ed);if(count($ed)>1)echo$this->admin->getLoginFormRow('driver',lang(28),html_select("auth[driver]",$ed,$cd).script("initLoginDriver(qsl('select'));",""));else
echo$this->admin->getLoginFormRow('driver','',input_hidden("auth[driver]",$cd));echo$this->admin->getLoginFormRow('server',lang(5),"<input class='input' name='auth[server]' value='".h($O)."' title='".lang(29)."' placeholder='localhost' autocapitalize='off'>");}echo$this->admin->getLoginFormRow('username',lang(6),'<input class="input" name="auth[username]" id="username" value="'.h($_GET["username"]).'" autocomplete="username" autocapitalize="off">'),$this->admin->getLoginFormRow('password',lang(30),'<input type="password" class="input" name="auth[password]" autocomplete="current-password">');if(!$Rk){$wc=isset($_GET["db"])?$_GET["db"]:$this->config->getDefaultDatabase();echo$this->admin->getLoginFormRow('db',lang(31),'<input class="input" name="auth[db]" value="'.h($wc).'" autocapitalize="off">');}echo"</table>\n","<p>","<input type='submit' class='button default' value='".lang(32)."'>",checkbox("auth[permanent]",1,$_COOKIE["neo_permanent"],lang(33)),"</p>\n";}function
getFieldName(array$k,$D=0){$U=$k["full_type"].($k["null"]?" NULL":"");$Ob=$k["comment"];$Nk=$U&&$Ob!=""?": ":"";return'<span title="'.h($U.$Nk.$Ob).'">'.h($k["field"]).'</span>';}function
printTableMenu(array$Rl,$Gf){echo'<p class="links top-tabs">';$Mg=[];$Jk=($this->settings->isSelectionPreferred()&&!$this->settings->isNavigationReversed())||(!$this->settings->isSelectionPreferred()&&$this->settings->isNavigationReversed());if($Jk)$Mg["select"]=[lang(34),"data"];if(support("table")||support("indexes"))$Mg["table"]=[lang(35),"structure"];if(!$Jk)$Mg["select"]=[lang(34),"data"];$Q=$Rl["Name"];$bg=false;if(support("table")){$bg=is_view($Rl);if(!$bg){if($Q!="")$Mg["create"]=[lang(36),"edit"];}elseif(support("view"))$Mg["view"]=[lang(37),"edit"];}if($Gf!==null)$Mg["edit"]=[lang(8),"item-add"];$Ti=$Gf?"&".http_build_query($Gf):"";foreach($Mg
as$u=>$X)echo" <a href='",h(ME),"$u=",urlencode($Q),($u=="edit"?$Ti:""),"'",bold(isset($_GET[$u])),">",icon($X[1]),"$X[0]</a>";echo
doc_link([DIALECT=>Driver::get()->tableHelp($Q,$bg)],icon("help").lang(38)),"\n";}function
formatSelectQuery($I,$tl,$Vd=false){$Il=support("sql");$_n=!$Vd?Driver::get()->warnings():null;if($Il)$I
.=";";$Ll=DIALECT=="elastic"||DIALECT=="mongo"?"json":DIALECT;$K="<pre><code class='jush-$Ll'>".h(str_replace("\n"," ",$I))."</code></pre>\n";$K
.="<p class='links'>";if($Il)$K
.="<a href='".h(ME)."sql=".urlencode($I)."'>".icon("edit").lang(39)."</a>";if($_n)$K
.="<a href='#warnings' class='toggle'>".lang(40).icon_chevron_down()."</a>";$K
.=" <span class='time'>(".format_time($tl).")</span>";$K
.="</p>\n";if($_n){$K
.=script("initToggles(qsl('p'));");$K
.="<div id='warnings' class='warnings hidden'>\n$_n\n</div>\n";}return$K;}function
formatMessageQuery($I,$qm,$Vd=false){restart_session();$ff=&get_session("queries");if(!isset($ff[$_GET["db"]]))$ff[$_GET["db"]]=[];if(strlen($I)>1e6)$I=preg_replace('~[\x80-\xFF]+$~','',substr($I,0,1e6))."\n…";$ff[$_GET["db"]][]=[$I,time(),$qm];$Il=support("sql");$_n=!$Vd?Driver::get()->warnings():null;$ol="sql-".count($ff[$_GET["db"]]);$An="warnings-".count($ff[$_GET["db"]]);$K=" ";if($_n)$K
.="<a href='#$An' class='toggle'>".lang(40).icon_chevron_down()."</a>, ";$Lj=support("sql")?lang(41):lang(42);$K
.="<a href='#$ol' class='toggle'>$Lj".icon_chevron_down()."</a>";$K
.=" <span class='time'>".@date("H:i:s")."</span>\n";if($_n)$K
.="<div id='$An' class='warnings hidden'>\n$_n</div>\n";$K
.="<div id='$ol' class='hidden'>\n";$Ll=DIALECT=="elastic"||DIALECT=="mongo"?"json":DIALECT;$K
.="<pre><code class='jush-$Ll'>".truncate_utf8($I,1000)."</code></pre>\n";$K
.="<p class='links'>";if($Il)$K
.="<a href='".h(str_replace("db=".urlencode(DB),"db=".urlencode($_GET["db"]),ME).'sql=&history='.(count($ff[$_GET["db"]])-1))."'>".icon("edit").lang(39)."</a>";if($qm)$K
.=" <span class='time'>($qm)</span>";$K
.="</p>\n";$K
.="</div>\n";return$K;}function
formatSqlCommandQuery($I){if(preg_match('~^DELIMITER\s~i',$I))return"";return
truncate_utf8($I,1000);}function
getTableDescriptionFieldName($Q){return"";}function
fillForeignDescriptions(array$M,array$se){return$M;}function
formatSelectionValue($X,$x,$k,$Li){if($X===null)$km="<i>NULL</i>";elseif(!$k)$km=$X;elseif(preg_match("~char|binary|boolean~",$k["type"])&&!preg_match("~var~",$k["type"]))$km="<code>$X</code>";elseif(is_blob($k)&&!is_utf8($X))$km="<i>".lang(43,strlen($Li))."</i>";elseif($this->admin->detectJson($k["full_type"],$Li))$km="<code class='jush-json'>$X</code>";else$km=$X;if($x)$km="<a href='".h($x)."'".(is_web_url($x)?target_blank():"").">$km</a>";return$km;}function
formatFieldValue($Y,array$k){return$Y;}function
printTableStructure(array$l){echo"<div class='scrollable'>\n","<table class='nowrap'>\n","<thead><tr>","<th>",lang(44),"</th>","<td>",lang(45)," <span class='note'>[".lang(46)."]</span></td>","<td>",lang(47),"</td>";if(support("comment"))echo"<td>",lang(48),"</td>";echo"</tr></thead>\n";$in=Driver::get()->getUserTypes();foreach($l
as$k){echo"<tr>","<th>",h($k["field"]),"</th>","<td>";$U=h($k["full_type"]);if(in_array($U,$in))echo"<a href='".h(ME.'type='.urlencode($U))."'>$U</a>";else
echo$U;if($k["null"])echo" <i>NULL</i>";if($k["auto_increment"])echo" <i>".lang(49)."</i>";if(isset($k["default"])){if($k["generated"])echo" <i>AS</i> <code class='jush-".DIALECT."'>".h(parenthesize_expression($k["default"]))."</code>"," <i>".h($k["generated"])."</i>";else
echo" [<strong>".h($k["default"])."</strong>]";}echo"</td>","<td>",h($k["collation"]),"</td>";if(support("comment"))echo"<td>",$this->admin->formatComment($k["comment"]),"</td>";echo"\n";}echo"</table>\n","</div>\n";}function
printTablePartitions(array$Yi){$bl=isset($Yi["partition_names"]);echo"<p>","<code class='jush-".DIALECT."'>BY ".h($Yi["partition_by"])." (".h($Yi["partition"]).")</code>";if(!$bl&&isset($Yi["partitions"]))echo" ".lang(50).": ".h($Yi["partitions"]);echo"</p>";if($bl){echo"<table>\n","<thead><tr><th>".lang(51)."</th><td>".lang(52)."</td></tr></thead>\n";foreach($Yi["partition_names"]as$u=>$A){echo"<tr><th>";if(DIALECT=="pgsql")echo"<a href='",h(ME."table=".urlencode($A)),"'>";echo
h($A);if(DIALECT=="pgsql")echo"</a>";echo"</th><td>".h($Yi["partition_values"][$u])."\n";}echo"</table>\n";}}function
printRelatedTables(array$S){echo"<ul class='links'>\n";foreach($S
as$L){$x=preg_replace('~ns=[^&]*~',"ns=".urlencode($L["ns"]),ME);echo"<li><a href='",h($x."table=".urlencode($L["table"])),"'>",icon("structure");if($L["ns"]!=$_GET["ns"])echo"<b>".h($L["ns"])."</b>.";echo
h($L["table"]),"</a>";}echo"</ul>\n";}function
printTableIndexes(array$t,array$Rl){$Bc=first(Driver::get()->getIndexAlgorithms($Rl));$Wi=false;foreach($t
as$s){if(isset($s["partial"])?$s["partial"]:false){$Wi=true;break;}}echo"<table>\n","<thead><tr>","<th>",lang(45),"</th>","<td>",lang(53)," <span class='note'>(",lang(54),")</span></td>";if($Wi)echo"<td>",lang(55),"</td>";echo"</tr></thead>\n";foreach($t
as$A=>$s){ksort($s["columns"]);$Bj=[];foreach($s["columns"]as$u=>$X)$Bj[]="<i>".h($X)."</i>".($s["lengths"][$u]?"(".h($s["lengths"][$u]).")":"").($s["descs"][$u]?" DESC":"");echo"<tr title='",h($A),"'>","<th>",h($s["type"]);if(isset($s['algorithm'])&&$s['algorithm']!=$Bc)echo" (",h($s['algorithm']),")";echo"</th>","<td>",implode(", ",$Bj),"</td>";if($Wi){echo"<td>";if($s['partial'])echo"<code class='jush-",DIALECT,"'>WHERE ",h($s['partial']),"</code>";echo"</td>";}echo"</tr>\n";}echo"</table>\n";}function
printSelectionColumns(array$N,array$c){print_fieldset_start("select",lang(56),"columns",(bool)$N,true);$N[""]=[];$q=0;foreach($N
as$u=>$X){$X=isset($_GET["columns"][$u])?$_GET["columns"][$u]:[];$b=select_input("name='columns[$q][col]'",$c,isset($X["col"])?$X["col"]:null,$u!==""?"selectFieldChange":"selectAddRow");echo"<div ",($u!=""?"":"class='no-sort'"),">",icon("handle","handle jsonly");if(Driver::get()->getFunctions()||Driver::get()->getGrouping())echo
html_select("columns[$q][fun]",[-1=>""]+array_filter([lang(57)=>Driver::get()->getFunctions(),lang(58)=>Driver::get()->getGrouping()]),isset($X["fun"])?$X["fun"]:null),help_script_command("value && value.replace(/ |\$/, '(') + ')'",true),script("qsl('select').onchange = (event) => { ".($u!==""?"":" qsl('select, input:not(.remove)', event.target.parentNode).onchange();")." };",""),"($b)";else
echo$b;echo" <button class='button light remove jsonly' title='",lang(59),"'>",icon_solo("remove"),"</button>",script("qsl('#fieldset-select .remove').onclick = selectRemoveRow;",""),"</div>\n";$q++;}print_fieldset_end("select",true);}function
printSelectionSearch(array$Z,array$c,array$t){print_fieldset_start("search",lang(60),"search",(bool)$Z);foreach($t
as$q=>$s){if($s["type"]=="FULLTEXT"){echo"<div>(<i>".implode("</i>, <i>",array_map('AdminNeo\h',$s["columns"]))."</i>) AGAINST","<input type='text' class='input' name='fulltext[".h($q)."]' value='".h(isset($_GET["fulltext"][$q])?$_GET["fulltext"][$q]:null)."'>",script("qsl('input').oninput = selectFieldChange;","");if(DIALECT=='sql')echo
checkbox("boolean[$q]",1,isset($_GET["boolean"][$q]),"BOOL");echo"</div>\n";}}$ob="this.parentNode.firstChild.onchange();";foreach(array_merge(array_values((array)$_GET["where"]),[[]])as$q=>$X){if(!$X||("$X[col]$X[val]"!=""&&in_array($X["op"],$this->getOperators())))echo"<div>",select_input(" name='where[$q][col]'",$c,$X["col"],($X?"selectFieldChange":"selectAddRow"),"(".lang(61).")"),html_select("where[$q][op]",$this->getOperators(),$X["op"],$ob),"<input type='text' class='input' name='where[$q][val]' value='".h($X["val"])."'>",script("mixin(qsl('input'), {oninput: function () { $ob }, onkeydown: selectSearchKeydown});","")," <button class='button light remove jsonly' title='".lang(59)."'>",icon_solo("remove"),"</button>",script('qsl("#fieldset-search .remove").onclick = selectRemoveRow;',""),"</div>\n";}print_fieldset_end("search");}function
printSelectionOrder(array$D,array$c,array$t){print_fieldset_start("sort",lang(62),"sort",(bool)$D,true);$_GET["order"][""]="";$q=0;foreach((array)$_GET["order"]as$u=>$X){if($u!=""&&$X=="")continue;echo"<div ",($u!=""?"":"class='no-sort'"),">",icon("handle","handle jsonly"),select_input("name='order[$q]'",$c,$X,$u!==""?"selectFieldChange":"selectAddRow")," ",checkbox("desc[$q]",1,isset($_GET["desc"][$u]),lang(63))," <button class='button light remove jsonly' title='",lang(59),"'>",icon_solo("remove"),"</button>",script('qsl("#fieldset-sort .remove").onclick = selectRemoveRow;',""),"</div>\n";$q++;}print_fieldset_end("sort",true);}function
printSelectionLimit($w){echo"<fieldset><legend>".lang(64)."</legend><div class='fieldset-content'>","<input type='number' name='limit' class='input size' value='".h($w?:"")."'>",script("qsl('input').oninput = selectFieldChange;",""),"</div></fieldset>\n";}function
printSelectionLength($lm){if($lm!==null)echo"<fieldset><legend>".lang(65)."</legend><div class='fieldset-content'>","<input type='number' name='text_length' class='input size' value='".h($lm)."'>","</div></fieldset>\n";}function
printSelectionAction(array$t){echo"<fieldset><legend>".lang(66)."</legend><div class='fieldset-content'>","<input type='submit' class='button' value='".lang(56)."'>"," <span id='noindex' title='".lang(67)."'></span>","<script".nonce().">\n";$c=new
stdClass();foreach($t
as$s){$pc=reset($s["columns"]);if($s["type"]!="FULLTEXT"&&$pc)$c->$pc=null;}echo"const indexColumns = ".json_encode($c,JSON_UNESCAPED_UNICODE|JSON_HEX_TAG).";\n","selectFieldChange.call(gid('form')['select']);\n","</script>\n","</div></fieldset>\n";}function
processSelectionColumns(array$c,array$t){$N=[];$Ne=[];foreach((array)$_GET["columns"]as$u=>$X){if($X["fun"]=="count"||($X["col"]!=""&&(!$X["fun"]||in_array($X["fun"],Driver::get()->getFunctions())||in_array($X["fun"],Driver::get()->getGrouping())))){$N[$u]=apply_sql_function($X["fun"],($X["col"]!=""?idf_escape($X["col"]):"*"));if(!in_array($X["fun"],Driver::get()->getGrouping()))$Ne[]=$N[$u];}}return[$N,$Ne];}function
processSelectionSearch(array$l,array$t){if(isset($_GET["where"]))$_GET["where"]=validate_search_conditions($_GET["where"]);$K=[];foreach($t
as$q=>$s){if($s["type"]=="FULLTEXT"&&isset($_GET["fulltext"])&&$_GET["fulltext"][$q]!="")$K[]="MATCH (".implode(", ",array_map('AdminNeo\idf_escape',$s["columns"])).") AGAINST (".q($_GET["fulltext"][$q]).(isset($_GET["boolean"][$q])?" IN BOOLEAN MODE":"").")";}foreach((array)$_GET["where"]as$Z){$Cb=$Z["col"];$qi=$Z["op"];$X=$Z["val"];if("$Cb$X"!=""&&in_array($qi,$this->getOperators())){if($qi=="SQL"&&(!$_POST||!verify_token()))Connection::get()->setUntrusted(true);$Vb=[];$Kf='-?\d+';$B='-?(\d+(\.\d*)?|\.\d+)';$Rf=preg_match('~IN$~',$qi);$Lf="~^$Kf".($Rf?"(,$Kf)*":"")."$~";$ai="~^$B".($Rf?"(,$B)*":"")."$~";$xc='~^(\d+)-(\d+)-(\d+)([ T]([01]?\d|2[0-3]):[0-5]\d(:[0-5]\d(\.\d+)?)?(\s*(z|utc|[+-](0?\d|1[0-5])(:?[0-5]\d)?))?)?$~i';foreach(($Cb!=""?[$Cb=>isset($l[$Cb])?$l[$Cb]:[]]:$l)as$A=>$k){$yj="";$Tb=" $qi";$Sf=preg_match('~'.number_type().(DIALECT=="pgsql"?"":'|bit').'~',$k["type"])&&!preg_match('~\[~',$k["full_type"]);$fi=DIALECT=="pgsql"&&$qi=="="&&$k["type"]=="oid";if($fi)$Tb
.=" ".$this->admin->processFieldInput($k,$X)."::regproc";elseif($Rf){$Tf=$Sf&&!(DIALECT=="pgsql"&&$k["type"]=="money");$tf=[];foreach(parse_search_list($X)as$Y)$tf[]=$Tf&&preg_match("~^$B$~",$Y)?$Y:q($Y);$Tb
.=" (".($tf?implode(", ",$tf):"NULL").")";}elseif($qi=="SQL")$Tb=" $X";elseif(preg_match('~^(I?LIKE) %%$~',$qi,$z))$Tb=" $z[1] ".$this->admin->processFieldInput($k,"%$X%");elseif($qi=="FIND_IN_SET"){$yj="$qi(".q($X).", ";$Tb=")";}elseif(!preg_match('~NULL$~',$qi))$Tb
.=" ".$this->admin->processFieldInput($k,$X);if($Cb!=""||(isset($k["privileges"]["where"])&&(preg_match(preg_match('~^'.int_type().'$~',$k["type"])?$Lf:$ai,$X)||!$Sf)&&(!preg_match("~[\x80-\xFF]~",$X)||preg_match('~char|text|enum|set|keyword~',$k["type"]))&&(!preg_match('~date|timestamp~',$k["type"])||(preg_match($xc,$X,$z)&&checkdate((int)$z[2],(int)$z[3],(int)$z[1])))&&Driver::get()->isSearchable($Z,$k)&&(!preg_match('~^elastic~',DRIVER)||$k["type"]!="boolean"||preg_match('~^(true|false)$~i',$X))&&(!preg_match('~^elastic~',DRIVER)||strpos($qi,"regexp")===false||preg_match('~text|keyword~',$k["type"]))&&(!preg_match('~^elastic~',DRIVER)||!preg_match('~^((unsigned_)?long|short|byte)$~',$k["type"])||preg_match('~^-?\d+$~',$X)))){if($fi)$Vb[]=$yj.idf_escape($A).$Tb;else$Vb[]=$yj.Driver::get()->convertSearch(idf_escape($A),$Z,$k).$Tb;}}if(count($Vb)==1)$K[]=$Vb[0];elseif($Vb)$K[]="(".implode(" OR ",$Vb).")";else$K[]="1 = 0";}}return$K;}function
processSelectionOrder(array$l,array$t){$K=[];$Ed=(DIALECT=="clickhouse"?'\\\\':'');$p='~^((COUNT\(DISTINCT |[A-Z0-9_]+\()(`(?:[^`'.$Ed.']|``)+`|"(?:[^"'.$Ed.']|"")+")\)|COUNT\(\*\))$~';foreach((array)$_GET["order"]as$u=>$X){if($X!="")$K[]=(preg_match($p,$X)?$X:idf_escape($X)).(isset($_GET["desc"][$u])?" DESC".(DIALECT=="pgsql"&&(isset($l[$X]["null"])?$l[$X]["null"]:null)?" NULLS LAST":""):"");}return$K;}function
processSelectionLength(){return
isset($_GET["text_length"])?$_GET["text_length"]:"100";}function
getFieldFunctions(array$k){$K=($k["null"]?"NULL/":"");$Ym=isset($_GET["select"])||where($_GET);foreach([Driver::get()->getInsertFunctions(),Driver::get()->getEditFunctions()]as$u=>$Fe){if(!$u||(!isset($_GET["call"])&&$Ym)){foreach($Fe
as$ij=>$X){if(!$ij||preg_match("~$ij~",$k["type"]))$K
.="/$X";}}if($u&&$Fe&&!preg_match('~enum|set|bool~',$k["type"])&&!is_blob($k))$K
.="/SQL";}if($k["auto_increment"]&&!$Ym)$K=lang(49);return
explode("/",$K);}function
getFieldInput($Q,array$k,$Na,$Y,$p){return"";}function
processFieldInput(array$k,$Y,$p=""){if($p=="SQL")return$Y;$A=$k["field"];$K=q($Y);if(preg_match('~^(now|getdate|uuid)$~',$p))$K="$p()";elseif(preg_match('~^current_(date|timestamp)$~',$p))$K=$p;elseif(preg_match('~^([+-]|\|\|)$~',$p))$K=idf_escape($A)." $p $K";elseif(preg_match('~^[+-] interval$~',$p))$K=idf_escape($A)." $p ".(preg_match("~^(\\d+|'[0-9.: -]') [A-Z_]+\$~i",$Y)&&DIALECT!="pgsql"?$Y:$K);elseif(preg_match('~^(addtime|subtime|concat)$~',$p))$K="$p(".idf_escape($A).", $K)";elseif(preg_match('~^(md5|sha1|password|encrypt)$~',$p))$K="$p($K)";elseif($k["type"]=="boolean"&&DIALECT=="elastic")$K=preg_match('~^(0|false)$~i',$Y)?"false":"true";return
unconvert_field($k,$K);}function
getDumpFormats(){return(support("dump")?['sql'=>'SQL']:[])+parent::getDumpFormats();}function
sendDumpFormatHeaders($of,$Dh=false){$Rd=(str_contains($_POST["format"],"sql")?"sql":($Dh?"tar":"csv"));if($Rd=="tar")header("Content-Type: application/x-tar");elseif($Rd=="sql"||$_POST["output"]=="text")header("Content-Type: text/plain; charset=utf-8");else
header("Content-Type: text/csv; charset=utf-8");return$Rd;}function
getDumpOutputs(){$Pi=['file'=>lang(68),'text'=>lang(69),];if(function_exists('gzencode'))$Pi['gz']='gzip';return$Pi;}function
sendDumpOutputHeaders($of,$Rd){if($_POST["output"]=="gz"){header("Content-Type: application/x-gzip");if(function_exists('deflate_init')){$cc=deflate_init(ZLIB_ENCODING_GZIP);ob_start(function($yl,$kj)use($cc){return
deflate_add($cc,$yl,($kj&PHP_OUTPUT_HANDLER_END?ZLIB_FINISH:ZLIB_NO_FLUSH));},1e6);}else{ob_start(function($yl){return
gzencode($yl);},1e6);}}}function
dumpTable($Q,$Cl,$tn=0){if($_POST["format"]!="sql"){echo"\xef\xbb\xbf";if($Cl)dump_csv(array_keys(fields($Q)));}else{if($tn==2){$l=[];foreach(fields($Q)as$A=>$k)$l[]=idf_escape($A)." $k[full_type]";$e="CREATE TABLE ".table($Q)." (".implode(", ",$l).")";}else$e=create_sql($Q,$_POST["auto_increment"],$Cl);set_utf8mb4($e);if($Cl&&$e){if($Cl=="DROP+CREATE"||$tn==1)echo"DROP ".($tn==2?"VIEW":"TABLE")." IF EXISTS ".table($Q).";\n";if($tn==1)$e=remove_definer($e);echo"$e;\n\n";}}}function
dumpData($Q,$Cl,$I){if($Cl){$dh=(DIALECT=="sqlite"?0:1048576);$l=[];$pf=false;if($_POST["format"]=="sql"){if($Cl=="TRUNCATE+INSERT")echo
truncate_sql($Q).";\n";$l=fields($Q);if(DIALECT=="mssql"){foreach($l
as$k){if($k["auto_increment"]){echo"SET IDENTITY_INSERT ".table($Q)." ON;\n";$pf=true;break;}}}}$J=Connection::get()->query($I,1);if($J){$Ef="";$gb="";$mg=[];$He=[];$Fl="";$jc=0;while($L=($Q!=''?$J->fetchAssoc():$J->fetchRow())){if(!$mg){$nn=[];foreach($L
as$X){$k=$J->fetchField();if(!empty($l[$k->name]['generated'])){$He[$k->name]=true;continue;}$mg[]=$k->name;$u=idf_escape($k->name);$nn[]="$u = VALUES($u)";}$Fl=($Cl=="INSERT+UPDATE"?"\nON DUPLICATE KEY UPDATE ".implode(", ",$nn):"").";\n";}if($_POST["format"]!="sql"){if($Cl=="table"){dump_csv($mg);$Cl="INSERT";}dump_csv($L);}else{if(!$Ef)$Ef="INSERT INTO ".table($Q)." (".implode(", ",array_map('AdminNeo\idf_escape',$mg)).") VALUES";foreach($L
as$u=>$X){if(isset($He[$u])){unset($L[$u]);continue;}$k=isset($l[$u])?$l[$u]:null;$L[$u]=($X===null?"NULL":($X===false?0:(!$k?(is_utf8($X)?q($X):Driver::get()->quoteBinary($X)):unconvert_field($k,preg_match(number_type(),$k["type"])&&!preg_match('~\[~',$k["full_type"])&&is_numeric($X)?$X:(!is_blob($k)||is_utf8($X)?q($X):Driver::get()->quoteBinary($X))))));}$sk=($dh?"\n":" ")."(".implode(",\t",$L).")";if(!$gb)$gb=$Ef.$sk;elseif(DIALECT=="mssql"?$jc%1000!=0:strlen($gb)+4+strlen($sk)+strlen($Fl)<$dh)$gb
.=",$sk";else{echo$gb.$Fl;$gb=$Ef.$sk;}}$jc++;}if($gb)echo$gb.$Fl;}elseif($_POST["format"]=="sql")echo"-- ".str_replace("\n"," ",Connection::get()->getError())."\n";if($pf)echo"SET IDENTITY_INSERT ".table($Q)." OFF;\n";}}function
getImportFilePath(){return"adminneo.sql";}function
printDatabaseMenu(){echo"<p class='links top-links'>\n";$Xh=isset($_GET["ns"])?$_GET["ns"]:null;if($Xh==""&&support("database"))echo'<a href="',h(ME),'database=">',icon("edit"),lang(70),"</a>\n";if($Xh!=""&&support("scheme"))echo"<a href='",h(ME),"scheme='>",icon("edit"),lang(71),"</a>\n";if($Xh!=="")echo'<a href="',h(ME),'schema=">',icon("schema"),lang(72),"</a>\n";if(support("privileges"))echo"<a href='",h(ME),"privileges='>",icon("users"),lang(73),"</a>\n";echo"</p>\n";}function
printNavigation($xh){if($xh=="auth"){$Oi="";foreach((array)$_SESSION["pwds"]as$pn=>$Vk){foreach($Vk
as$O=>$jn){foreach($jn
as$V=>$F){if($F!==null){$_c=$_SESSION["db"][$pn][$O][$V];foreach(($_c?array_keys($_c):[""])as$h){$Sk=$this->admin->getServerName($O,false);$T=h(get_driver_name($pn,$O)).($V!=""||$Sk!=""?" - ":"").h($V).($V!=""&&$Sk!=""?"@":"").h($Sk).($h!=""?h(" - $h"):"");$Oi
.="<li><a href='".h(auth_url($pn,$O,$V,$h))."' class='primary' title='$T'>$T</a></li>\n";}}}}}if($Oi)echo"<nav id='logins'><menu>\n$Oi</menu></nav>\n";}else{$this->admin->printDatabaseSwitcher($xh);$va=[];if(DB==""||!$xh){if(support("sql")){$va[]="<a href='".h(ME)."sql='".bold(isset($_GET["sql"])&&!isset($_GET["import"])).">".icon("command").lang(41)."</a>";$va[]="<a href='".h(ME)."import='".bold(isset($_GET["import"])).">".icon("import").lang(74)."</a>";}$va[]="<a href='".h(ME)."dump=".urlencode(isset($_GET["table"])?$_GET["table"]:$_GET["select"])."' id='dump'".bold(isset($_GET["dump"])).">".icon("export").lang(75)."</a>";}if(DB=="")$va[]='<a href="'.h(ME).'database="'.bold($_GET["database"]==="").">".icon("database-add").lang(76)."</a>\n";if(DB!=""&&$_GET["ns"]===""&&!$xh)$va[]='<a href="'.h(ME).'scheme="'.bold($_GET["scheme"]==="").">".icon("database-add").lang(77)."</a>\n";if(DB!=""&&$_GET["ns"]!==""&&!$xh)$va[]='<a href="'.h(ME).'create="'.bold($_GET["create"]==="").">".icon("table-add").lang(78)."</a>\n";if($va)echo"<p class='links'>".implode("\n",$va)."</p>";$S=[];if($_GET["ns"]!==""&&!$xh&&DB!=""){Connection::get()->selectDatabase(DB);$S=table_status('',true);}if($_GET["ns"]!==""&&!$xh&&DB!=""){if($S){$this->admin->printTablesFilter();$this->admin->printTableList($S);}else
echo"<div id='tables'><p>".lang(79)."</p></div>\n";}elseif($_GET["ns"]!==""&&DB!="")echo"<div id='tables'></div>\n";if(support("sql")||DIALECT=="elastic"||DIALECT=="mongo"){echo"<script".nonce().">\n";if(support("sql")&&$S){$Mg=[];foreach($S
as$Q=>$U)$Mg[]=js_escape_re($Q);$Ql=support("table")&&!$this->config->isSelectionPreferred()?"table":"select";echo"window.jushLinks = { ".DIALECT.": {\n",js_escape_key(ME.$Ql.'=$&'),': /\b(?<!\$)('.implode('|',$Mg).')(?!\$)\b/g';$pl=["sql","check","event","procedure","trigger","view","type","table","processlist"];if(support('routine')&&array_intersect_key($_GET,array_flip($pl))){foreach(routines()as$L)echo",\n",js_escape_key(ME.'function='.urlencode($L["SPECIFIC_NAME"]).'&name=$&'),': /\b'.js_escape_re($L["ROUTINE_NAME"]).'(?=["`\]]?\()/g';}echo"\n}};\n";foreach(["bac","bra","sqlite_quo","mssql_bra"]as$X)echo"jushLinks.$X = jushLinks.".DIALECT.";\n";}if(DIALECT!="elastic"&&DIALECT!="mongo"&&$this->getConfig()->isSqlAutocompletionEnabled()&&(isset($_GET["sql"])||isset($_GET["trigger"])||isset($_GET["check"]))){$am=array_fill_keys(array_keys($S),[]);foreach(Driver::get()->getAllFields()as$Q=>$l){foreach($l
as$k)$am[$Q][]=$k["field"];}echo"window.addEventListener('DOMContentLoaded', () => { autocompletion = jush.autocompleteSql('".idf_escape("")."', ".json_encode($am,JSON_HEX_TAG)."); });\n";}echo"</script>\n";}echo
script("let autocompletion;\nwindow.addEventListener('DOMContentLoaded', () => { initSyntaxHighlighting('".js_escape(doc_version())."', '".js_escape(Connection::get()->getFlavor())."', autocompletion); });");}}function
printDatabaseSwitcher($xh){$g=$this->admin->getDatabases();if(!$g&&DIALECT!="sqlite")return;echo"<div class='db-selector'><form action=''>";hidden_fields_get();echo"<div>";if($g)echo"<select id='database-select' name='db' title='",lang(31),"'>".optionlist([""=>"(".lang(80).")"]+$g,DB)."</select>".script("mixin(gid('database-select'), {onmousedown: dbMouseDown, onchange: dbChange});");else
echo"<input id='database-select' class='input' name='db' value='".h(DB)."' title='",lang(31),"' autocapitalize='off'>\n";echo"<input type='submit' value='".lang(81)."' class='button ".($g?"hidden":"")."'>\n","</div>";foreach(["import","sql","schema","dump","privileges"]as$X){if(isset($_GET[$X])){echo
input_hidden($X);break;}}echo"</form></div>\n";}function
printTableList(array$S){$jd=$this->settings->isNavigationDual()||$this->settings->isNavigationHover();$mh=($jd?"class='dual".($this->settings->isNavigationHover()?" hover":"")."'":($this->settings->isNavigationReversed()?"class='reversed'":""));echo"<nav id='tables'><div class='scroll-marker'></div><menu $mh>";foreach($S
as$Q=>$vl){$Q="$Q";$A=$this->admin->getTableName($vl);if($A==""||(isset($vl["Partition"])?$vl["Partition"]:false))continue;echo"<li>";$wa=in_array($Q,[$_GET["table"],$_GET["select"],$_GET["create"],$_GET["indexes"],$_GET["foreign"],$_GET["trigger"],$_GET["check"],$_GET["view"]]);$Ab="primary".(is_view($vl)?" view":"");$Jl=support("table")||support("indexes");$Fk=h(ME)."select=".urlencode($Q);$Sl=h(ME)."table=".urlencode($Q);if($this->settings->isSelectionPreferred()){if($this->settings->isNavigationReversed()&&$Jl)echo" <a href='$Sl' title='",lang(35),"' class='secondary'>",icon("structure"),"</a>";echo"<a href='$Fk'",bold($wa,$Ab)," data-primary='true' title='$A'>$A</a>";if($jd&&$Jl)echo" <a href='$Sl' title='",lang(35),"' class='secondary'>",icon_solo("structure"),"</a>";}else{if($this->settings->isNavigationReversed())echo" <a href='$Fk' title='",lang(34),"' class='secondary'>",icon("data"),"</a>";if($Jl)echo"<a href='$Sl'",bold($wa,$Ab)," data-primary='true' title='$A'>$A</a>";else
echo"<span data-primary='true'",bold($wa,$Ab),">$A</span>";if($jd)echo" <a href='$Fk' title='",lang(34),"' class='secondary'>",icon_solo("data"),"</a>";}echo"</li>\n";}echo"</menu></nav>\n",script("initTablesList(".json_encode($this->admin->getDatabase(),JSON_HEX_TAG).");");}function
getSettingsRows($Pe){$P=parent::getSettingsRows($Pe);if($Pe==1){$C=[""=>lang(11),Config::$NavigationSimple=>lang(82),Config::$NavigationDual=>lang(83),Config::$NavigationHover=>lang(84),Config::$NavigationReversed=>lang(85)];$i=$C[$this->config->getNavigationMode()];$C[""].=" ($i)";$P["navigationMode"]="<tr><th>".lang(86)."</th>"."<td>".html_radios("navigationMode",$C,($ra=$this->settings->getParameter("navigationMode"))!==null?$ra:"")."<span class='input-hint'>".lang(87)."</span>"."</td></tr>\n";$C=[""=>lang(11),0=>lang(35),1=>lang(34),];$i=$C[$this->config->isSelectionPreferred()?1:0];$C[""].=" ($i)";$P["preferSelection"]="<tr><th id='label-links'>".lang(88)."</th>"."<td>".html_select("preferSelection",$C,($ra=$this->settings->getParameter("preferSelection"))!==null?$ra:"","","label-links",true)."<span class='input-hint'>".lang(89)."</span>"."</td></tr>\n";}return$P;}function
getForeignColumnInfo(array$se,$b){return
null;}}class
TmpFile{private$handler;private$size;function
__construct(){$this->handler=tmpfile();}function
getSize(){return$this->size;}function
write($bc){if(!$this->handler)return;$this->size+=strlen($bc);fwrite($this->handler,$bc);}function
send(){if(!$this->handler)return;fseek($this->handler,0);fpassthru($this->handler);fclose($this->handler);}}function
print_select_result(Result$J,$d=null,array$Fi=[],$w=0){$Mg=[];$t=[];$c=[];$cb=[];$Mm=[];$K=[];for($q=0;(!$w||$q<$w)&&($L=$J->fetchRow());$q++){if(!$q){echo"<div class='scrollable'>\n","<table class='nowrap'>\n","<thead><tr>";for($eg=0;$eg<count($L);$eg++){$k=$J->fetchField();if(!$k){echo"<th></th>";continue;}$A=$k->name;$Ei=isset($k->orgtable)?$k->orgtable:"";$Di=isset($k->orgname)?$k->orgname:$A;if(isset($k->table))$K[$k->table]=$Ei;if($Fi&&DIALECT=="sql")$Mg[$eg]=($A=="table"?"table=":($A=="possible_keys"?"indexes=":null));elseif($Ei!=""){if(!isset($t[$Ei])){$t[$Ei]=[];foreach(indexes($Ei,$d)as$s){if($s["type"]=="PRIMARY"){$t[$Ei]=array_flip($s["columns"]);break;}}$c[$Ei]=$t[$Ei];}if(isset($c[$Ei][$Di])){unset($c[$Ei][$Di]);$t[$Ei][$Di]=$eg;$Mg[$eg]=$Ei;}}if($k->charsetnr==63)$cb[$eg]=true;$Mm[$eg]=$k->type;$T=trim(($Ei!=""?"$Ei.$Di":($k->name!=$Di?$Di:""))." ".Driver::get()->getTypeName($k));echo"<th".($T!=""?" title='".h($T)."'":"").">".h($A).($Fi?doc_link(['sql'=>"explain-output.html#explain_".strtolower($A),'mariadb'=>"reference/sql-statements/administrative-sql-statements/analyze-and-explain-statements/explain#columns-in-explain-...-select",]):"");}echo"</thead>\n";}echo"<tr>";foreach($L
as$u=>$X){$x="";if(isset($Mg[$u])&&!$c[$Mg[$u]]){if($Fi&&DIALECT=="sql"){$Q=$L[array_search("table=",$Mg)];$x=ME.$Mg[$u].urlencode($Fi[$Q]!=""?$Fi[$Q]:$Q);}else{$x=ME."edit=".urlencode($Mg[$u]);foreach($t[$Mg[$u]]as$Cb=>$eg)$x
.="&where".urlencode("[".bracket_escape($Cb)."]")."=".urlencode($L[$eg]);}}$U=($cb[$u]?'blob':($Mm[$u]==254?'char':''));$k=['full_type'=>$U,'type'=>$U,];$X=select_value($X,$x,$k,null);$Ab=$Mm[$u]<=9||$Mm[$u]==246?"class='number'":"";echo"<td $Ab>$X</td>";}}if($q)echo"</table>\n</div>";else
echo"<p class='message'>".lang(90);echo"\n";return$K;}function
referencable_primary($Lk){$K=[];foreach(table_status('',true)as$Ul=>$Q){if($Ul!=$Lk&&fk_support($Q)){foreach(fields($Ul)as$k){if($k["primary"]){if($K[$Ul]){unset($K[$Ul]);break;}$K[$Ul]=$k;}}}}return$K;}function
textarea($A,$Y,$M=10,$Jb=80){echo"<textarea name='".h($A)."' rows='$M' cols='$Jb' class='sqlarea jush-".DIALECT."' spellcheck='false' wrap='off'>";if(is_array($Y)){foreach($Y
as$X)echo
h($X[0])."\n\n\n";}else
echo
h($Y);echo"</textarea>";}function
select_input($Na,$C,$Y="",$oi="",$lj=""){if($C&&$Y!=""&&!isset($C[$Y]))$C=[$Y=>$Y]+$C;$em=($C?"select":"input");return"<$em $Na".($C?"><option value=''>$lj".optionlist($C,$Y,true)."</select>":" size='10' value='".h($Y)."' placeholder='$lj'>").($oi?script("qsl('$em').onchange = $oi;",""):"");}function
json_row($u,$X=null){static$le=true;if($le)echo"{";if($u!=""){echo($le?"":",")."\n\t\"".addcslashes($u,"\r\n\t\"\\/").'": '.($X!==null?'"'.addcslashes($X,"\r\n\t\"\\/").'"':'null');$le=false;}else{echo"\n}\n";$le=true;}}function
edit_type($u,$k,$Fb,$te=[],$Ud=[]){$U=isset($k["type"])?$k["type"]:null;echo'<td><select name="',h($u),'[type]" class="type" aria-labelledby="label-type">';$dd=Driver::get()->getTypes();if($U&&!isset($dd[$U])&&!isset($te[$U])&&!in_array($U,$Ud))$Ud[]=$U;$Bl=Driver::get()->getStructuredTypes();if($te)$Bl[lang(91)]=$te;echo
optionlist(array_merge($Ud,$Bl),$U),'</select><td><input name="',h($u),'[length]" value="',h(isset($k["length"])?$k["length"]:null),'" size="3"',(!(isset($k["length"])?$k["length"]:null)&&preg_match('~var(char|binary)$~',$U)?" class='input required'":" class='input'"),' aria-labelledby="label-length"><td class="options">',($Fb?"<select name='".h($u)."[collation]'".option_types($U,'(char|text|enum|set)$').'><option value="">('.lang(92).')'.optionlist($Fb,isset($k["collation"])?$k["collation"]:null).'</select>':''),(Driver::get()->getUnsigned()?"<select name='".h($u)."[unsigned]'".option_types($U,'^$|'.number_type()).'><option>'.optionlist(Driver::get()->getUnsigned(),isset($k["unsigned"])?$k["unsigned"]:null).'</select>':''),(isset($k['on_update'])?"<select name='".h($u)."[on_update]'".option_types($U,'timestamp|datetime').'>'.optionlist([""=>"(".lang(93).")","CURRENT_TIMESTAMP"],(preg_match('~^CURRENT_TIMESTAMP~i',$k["on_update"])?"CURRENT_TIMESTAMP":$k["on_update"])).'</select>':''),($te?"<select name='".h($u)."[on_delete]'".option_types($U,'`')."><option value=''>(".lang(94).")".optionlist(Driver::get()->getOnActions(),isset($k["on_delete"])?$k["on_delete"]:null)."</select> ":" ");}function
option_types($U,$Mm){return" data-types='".h($Mm)."'".(preg_match("~$Mm~",$U)?"":" class='hidden'");}function
process_length($v){if(preg_match('~^\s*\(?\s*max\s*\)?\s*$~i',$v))return"(max)";$_d=Driver::$EnumLengthPattern;return(preg_match("~^\\s*\\(?\\s*$_d(?:\\s*,\\s*$_d)*+\\s*\\)?\\s*\$~",$v)&&preg_match_all("~$_d~",$v,$_)?"(".implode(",",$_[0]).")":preg_replace('~^[-+.0-9].*~','(\0)',preg_replace('~[^-0-9,.+()[\]]~','',$v)));}function
process_type($k,$Db="COLLATE"){return" $k[type]".process_length($k["length"]).(preg_match(number_type(),$k["type"])&&in_array($k["unsigned"],Driver::get()->getUnsigned())?" $k[unsigned]":"").(preg_match('~char|text|enum|set~',$k["type"])&&$k["collation"]?" $Db ".(DIALECT=="mssql"?$k["collation"]:q($k["collation"])):"");}function
process_field($k,$Jm){if($k["on_update"])$k["on_update"]=preg_replace('~current_timestamp(\(\))?~i',"CURRENT_TIMESTAMP",$k["on_update"]);return[idf_escape(trim($k["field"])),process_type($Jm),($k["null"]?" NULL":" NOT NULL"),default_value($k),(preg_match('~timestamp|datetime~',$k["type"])&&$k["on_update"]?" ON UPDATE ".$k["on_update"]:""),(support("comment")&&$k["comment"]!=""?" COMMENT ".q(normalize_newlines($k["comment"])):""),($k["auto_increment"]?auto_increment():null),];}function
normalize_newlines($Y){return
str_replace("\r","",(string)$Y);}function
default_value($k){if($k["default"]===null)return"";$i=normalize_newlines($k["default"]);$Ge=$k["generated"];if(in_array($Ge,Driver::get()->getGenerated())){if(DIALECT=="mssql")return" AS ($i)".($Ge=="VIRTUAL"?"":" $Ge");else
return" GENERATED ALWAYS AS ($i) $Ge";}if(stripos($i,"GENERATED ")===0)return" $i";if(preg_match('~char|binary|text|json|enum|set~',$k["type"])||preg_match('~^(?![a-z])~i',$i)){if(DIALECT=="sql"&&preg_match('~text|json~',$k["type"]))return" DEFAULT (".q($i).")";else
return" DEFAULT ".q($i);}else{$i=str_ireplace("current_timestamp()","CURRENT_TIMESTAMP",$i);return" DEFAULT ".(DIALECT=="sqlite"?"($i)":$i);}}function
type_class($U){foreach(['char'=>'text','date'=>'time|year','binary'=>'blob','enum'=>'set',]as$Ab=>$ij){if(preg_match("~$Ab|$ij~",$U))return"class='$Ab'";}return"";}function
edit_fields(array$l,array$Fb,$U="TABLE",$te=[]){$l=array_values($l);$Rb=$_POST?$_POST["comments"]:Admin::get()->getSettings()->getParameter("commentsOpened");$Pb=$Rb?"":"class='hidden'";echo"<thead><tr>\n";if(support("move_col"))echo"<th class='jsonly'></th>";if($U=="PROCEDURE")echo"<td></td>";echo"<th id='label-name'>",($U=="TABLE"?lang(95):lang(96)),"</th>\n","<td id='label-type'>",lang(45),"<textarea id='enum-edit' rows='4' cols='12' wrap='off' hidden></textarea>",script("gid('enum-edit').onblur = onFieldLengthBlur;"),"</td>\n","<td id='label-length'>",lang(97),"</td>\n","<td>",lang(98),"</td>\n";if($U=="TABLE")echo"<td id='label-null'>NULL</td>\n","<td><input type='radio' name='auto_increment_col' value=''><abbr id='label-ai' title='",lang(49),"'>AI</abbr>",doc_link(['sql'=>"example-auto-increment.html",'mariadb'=>"reference/data-types/auto_increment",]),"</td>\n","<td id='label-default'>",lang(99),"</td>\n",support("comment")?"<td id='label-comment' $Pb>".lang(48)."</td>\n":"";echo"<td>","<button name='add[",(support("move_col")?0:count($l)),"]' value='1' title='",lang(100),"' class='button light'>",icon_solo("add"),"</button>",(support("move_col")?"":script("qsl('button').onclick = onAddLastFieldRowClick;")),script("row_count = ".count($l).";"),"</td>\n","</tr></thead>\n";$Ab=support("move_col")?"class='sortable'":"";echo"<tbody $Ab>\n";foreach($l
as$q=>$k){$q++;$Gi=$k[($_POST?"orig":"field")];$Tc=(isset($_POST["add"][$q-1])||(isset($k["field"])&&!(isset($_POST["drop_col"][$q])?$_POST["drop_col"][$q]:null)))&&(support("drop_col")||$Gi=="");echo"<tr",($Tc?"":" hidden"),">\n";if(support("move_col"))echo"<th class='handle jsonly'>",icon_solo("handle"),"</th>";if($U=="PROCEDURE")echo"<td>",html_select("fields[$q][inout]",Driver::get()->getInOut(),$k["inout"]),"</td>\n";echo"<th>";if($Tc)echo"<input class='input' name='fields[$q][field]' value='",h($k["field"]),"' data-maxlength='64' autocapitalize='off' aria-labelledby='label-name' ".(isset($_POST["add"][$q-1])?"autofocus":"").">";echo
input_hidden("fields[$q][orig]",$Gi);edit_type("fields[$q]",$k,$Fb,$te);echo"</th>\n";if($U=="TABLE"){echo"<td>",checkbox("fields[$q][null]",1,$k["null"],"","","block","label-null"),"</td>\n";$wb=$k["auto_increment"]?"checked":"";echo"<td><label class='block'><input type='radio' name='auto_increment_col' value='$q' $wb aria-labelledby='label-ai'></label></td>\n","<td class='default-value'>";if(Driver::get()->getGenerated())echo
html_select("fields[$q][generated]",array_merge(["","DEFAULT"],Driver::get()->getGenerated()),$k["generated"]);else
echo
checkbox("fields[$q][generated]",1,$k["generated"],"","","","label-default");$Na="name='fields[$q][default]' aria-labelledby='label-default'";$Y=h($k["default"]);if(str_contains($Y,"\n")){if($Y[0]=="\n")$Y="\n$Y";echo"<textarea $Na rows='3' cols='30' style='vertical-align: bottom;'>$Y</textarea>";}else
echo"<input class='input' $Na value='$Y'>";echo"</td>\n";if(support("comment")){$ch=Connection::get()->isMinVersion("5.5")?1024:255;$Na="name='fields[$q][comment]' data-maxlength='$ch' aria-labelledby='label-comment'";$Y=h($k["comment"]);echo"<td $Pb>";if(str_contains($Y,"\n")){if($Y[0]=="\n")$Y="\n$Y";echo"<textarea $Na rows='3' cols='30' style='vertical-align: bottom;'>$Y</textarea>";}else
echo"<input class='input' $Na value='$Y'>";echo"</td>\n";}}echo"<td>";if(support("move_col"))echo"<button name='add[$q]' value='1' title='".lang(100)."' class='button light'>",icon_solo("add"),"</button>","<button name='up[$q]' value='1' title='".lang(101)."' class='button light hidden'>",icon_solo("arrow-up"),"</button>","<button name='down[$q]' value='1' title='".lang(102)."' class='button light hidden'>",icon_solo("arrow-down"),"</button>";if($Gi==""||support("drop_col"))echo"<button name='drop_col[$q]' value='1' title='".lang(59)."' class='button light'>",icon_solo("remove"),"</button>";echo"</td>\n</tr>\n";}echo"</tbody>";}function
process_fields(&$l){$di=0;if($_POST["up"]){$xg=0;foreach($l
as$u=>$k){if(key($_POST["up"])==$u){unset($l[$u]);array_splice($l,$xg,0,[$k]);break;}if(isset($k["field"]))$xg=$di;$di++;}}elseif($_POST["down"]){$ze=false;foreach($l
as$u=>$k){if(isset($k["field"])&&$ze){unset($l[key($_POST["down"])]);array_splice($l,$di,0,[$ze]);break;}if(key($_POST["down"])==$u)$ze=$k;$di++;}}elseif($_POST["add"]){$l=array_values($l);array_splice($l,key($_POST["add"]),0,[[]]);}elseif(!$_POST["drop_col"])return
false;return
true;}function
normalize_enum($z){$X=$z[0];return"'".str_replace("'","''",addcslashes(stripcslashes(str_replace($X[0].$X[0],$X[0],substr($X,1,-1))),'\\'))."'";}function
grant($Ke,array$Ej,$c,$mi,$gn){if(!$Ej)return
true;if($Ej==["ALL PRIVILEGES","GRANT OPTION"]){if($Ke)return(bool)queries("GRANT ALL PRIVILEGES ON $mi TO $gn WITH GRANT OPTION");else
return
queries("REVOKE ALL PRIVILEGES ON $mi FROM $gn")&&queries("REVOKE GRANT OPTION ON $mi FROM $gn");}if($Ej==["GRANT OPTION","PROXY"]){if($Ke)return(bool)queries("GRANT PROXY ON $mi TO $gn WITH GRANT OPTION");else
return(bool)queries("REVOKE PROXY ON $mi FROM $gn");}return(bool)queries(($Ke?"GRANT ":"REVOKE ").preg_replace('~(GRANT OPTION)\([^)]*\)~','$1',implode("$c, ",$Ej).$c)." ON $mi ".($Ke?"TO ":"FROM ").$gn);}function
drop_create($fd,$e,$gd,$jm,$hd,$y,$qh,$oh,$ph,$ki,$Rh){if($_POST["drop"])query_redirect($fd,$y,$qh);elseif($ki=="")query_redirect($e,$y,$ph);elseif(support("transaction_ddl")){Driver::get()->begin();queries_redirect($y,$oh,queries($fd)&&queries($e)&&Driver::get()->commit());Driver::get()->rollback();}elseif($ki!=$Rh){$mc=queries($e);queries_redirect($y,$oh,$mc&&queries($fd));if($mc)queries($gd);}else
queries_redirect($y,$oh,queries($jm)&&queries($hd)&&queries($fd)&&queries($e));}function
create_trigger($mi,array$Em){$sm=" $Em[Timing] $Em[Event]".(preg_match('~ OF~',$Em["Event"])?" $Em[Of]":"");return"CREATE TRIGGER ".idf_escape($Em["Trigger"]).(DIALECT=="mssql"?$mi.$sm:$sm.$mi).rtrim(" $Em[Type]\n$Em[Statement]",";").";";}function
create_routine($ok,$L){$Yk=[];$l=(array)$L["fields"];ksort($l);$uf=implode("|",Driver::get()->getInOut());foreach($l
as$k){if($k["field"]!="")$Yk[]=(preg_match("~^($uf)\$~",$k["inout"])?"$k[inout] ":"").idf_escape($k["field"]).process_type($k,"CHARACTER SET");}$Fc=rtrim($L["definition"],";");return"CREATE $ok ".idf_escape(trim($L["name"]))." (".implode(", ",$Yk).")".($ok=="FUNCTION"?" RETURNS".process_type($L["returns"],"CHARACTER SET"):"").($L["language"]?" LANGUAGE $L[language]":"").(DIALECT=="pgsql"?" AS ".q($Fc):"\n$Fc;");}function
remove_definer($I){return
preg_replace('~^([A-Z =]+) DEFINER=`'.preg_replace('~@(.*)~','`@`(%|\1)',logged_user()).'`~','\1',$I);}function
format_foreign_key($o){$ni=implode("|",Driver::get()->getOnActions());$h=$o["db"];$Xh=$o["ns"];return" FOREIGN KEY (".implode(", ",array_map('AdminNeo\idf_escape',$o["source"])).") REFERENCES ".($h!=""&&$h!=$_GET["db"]?idf_escape($h).".":"").($Xh!=""&&$Xh!=$_GET["ns"]?idf_escape($Xh).".":"").idf_escape($o["table"])." (".implode(", ",array_map('AdminNeo\idf_escape',$o["target"])).")".(preg_match("~^($ni)\$~",$o["on_delete"])?" ON DELETE $o[on_delete]":"").(preg_match("~^($ni)\$~",$o["on_update"])?" ON UPDATE $o[on_update]":"").(isset($o["deferrable"])?" $o[deferrable]":"");}function
tar_file($n,TmpFile$wm){$Ze=pack("a100a8a8a8a12a12",$n,644,0,0,decoct($wm->getSize()),decoct(time()));$yb=8*32;for($q=0;$q<strlen($Ze);$q++)$yb+=ord($Ze[$q]);$Ze
.=sprintf("%06o",$yb)."\0 ";echo$Ze,str_repeat("\0",512-strlen($Ze));$wm->send();echo
str_repeat("\0",511-($wm->getSize()+511)%512);}function
doc_link(array$hj,$km="<sup>?</sup>"){if(!(isset($hj[DIALECT])?$hj[DIALECT]:null))return"";$qn=doc_version();$bn=['sql'=>"https://dev.mysql.com/doc/refman/$qn/en/",'sqlite'=>"https://www.sqlite.org/",'pgsql'=>"https://www.postgresql.org/docs/".(Connection::get()->isCockroachDB()?"current":$qn)."/",'mssql'=>"https://learn.microsoft.com/en-us/sql/",'oracle'=>"https://www.oracle.com/pls/topic/lookup?ctx=db".str_replace(".","",$qn)."&id=",'elastic'=>"https://www.elastic.co/guide/en/elasticsearch/reference/$qn/",];if(Connection::get()->isMariaDB()){$bn['sql']="https://mariadb.com/docs/server/";$hj['sql']=isset($hj['mariadb'])?$hj['mariadb']:str_replace(".html","",$hj['sql']);}return"<a href='".h($bn[DIALECT].$hj[DIALECT].(DIALECT=='mssql'?"?view=sql-server-ver$qn":""))."'".target_blank().">$km</a>";}function
doc_version(){return
preg_replace('~^(\d\.?\d).*~s','\1',Connection::get()->getVersion());}function
db_size($h){if(!Connection::get()->selectDatabase($h))return"?";$K=0;foreach(table_status()as$R)$K+=$R["Data_length"]+$R["Index_length"];return
format_number($K);}function
set_utf8mb4($e){static$Yk=false;if(!$Yk&&preg_match('~\butf8mb4~i',$e)){$Yk=true;echo"SET NAMES ".charset(Connection::get()).";\n\n";}}error_reporting(E_ALL&~E_DEPRECATED);set_error_handler(function($Bd,$j){return(bool)preg_match('~^Undefined (array key|offset|index)~',$j);},E_WARNING|E_NOTICE);;$ie=!preg_match('~^(unsafe_raw)?$~',ini_get("filter.default"));if($ie||ini_get("filter.default_flags")){foreach(['_GET','_POST','_COOKIE','_SERVER']as$X){$Tm=filter_input_array(constant("INPUT$X"),FILTER_UNSAFE_RAW);if($Tm)$$X=$Tm;}}foreach(["file","ext","username","db","ns"]as$u){if(isset($_GET[$u])&&!is_string($_GET[$u])){http_response_code(400);exit;}}if(function_exists("mb_internal_encoding"))mb_internal_encoding("8bit");class
Server{private$params;private$key;function
__construct(array$Ti,$u=null){$this->params=$Ti;$this->key=$u;}function
getKey(){return
isset($this->key)?$this->key:substr(md5($this->getDriver().$this->getServer()),0,8);}function
getDriver(){return$this->params["driver"];}function
getServer(){return
isset($this->params["server"])?$this->params["server"]:"";}function
getDatabase(){return
isset($this->params["database"])?$this->params["database"]:"";}function
getName(){return
isset($this->params["name"])?$this->params["name"]:(isset($this->params["server"])?$this->params["server"]:"");}function
getUsername(){return
isset($this->params["username"])?$this->params["username"]:"";}function
getPassword(){return
isset($this->params["password"])?$this->params["password"]:"";}function
hasCredentials(){return$this->getUsername()!=""||$this->getPassword()!="";}function
getConfigParams(){$Ti=isset($this->params["config"])?$this->params["config"]:[];$Je=["servers"];foreach($Je
as$Si){if(isset($Ti[$Si]))unset($Ti[$Si]);}return$Ti;}}class
Config{static$NavigationSimple="simple";static$NavigationDual="dual";static$NavigationHover="hover";static$NavigationReversed="reversed";private$params;private$servers=[];function
__construct(array$Ti){$this->params=$Ti;if(isset($this->params["servers"])){foreach($this->params["servers"]as$u=>$O){$Qk=new
Server($O,is_string($u)?$u:null);$this->params["servers"][$u]=$Qk;$this->servers[$Qk->getKey()]=$Qk;}}}function
getTheme(){return
isset($this->params["theme"])?$this->params["theme"]:"default";}function
getColorVariant(){return
isset($this->params["colorVariant"])?$this->params["colorVariant"]:"blue";}function
getCssUrls(){return$this->parseList(isset($this->params["cssUrls"])?$this->params["cssUrls"]:[]);}function
getJsUrls(){return$this->parseList(isset($this->params["jsUrls"])?$this->params["jsUrls"]:[]);}function
getNavigationMode(){return
isset($this->params["navigationMode"])?$this->params["navigationMode"]:self::$NavigationSimple;}function
isNavigationSimple(){return$this->getNavigationMode()==self::$NavigationSimple;}function
isNavigationDual(){return$this->getNavigationMode()==self::$NavigationDual;}function
isNavigationReversed(){return$this->getNavigationMode()==self::$NavigationReversed;}function
isSelectionPreferred(){return
isset($this->params["preferSelection"])?$this->params["preferSelection"]:false;}function
isJsonValuesDetection(){return
isset($this->params["jsonValuesDetection"])?$this->params["jsonValuesDetection"]:false;}function
isJsonValuesAutoFormat(){return
isset($this->params["jsonValuesAutoFormat"])?$this->params["jsonValuesAutoFormat"]:false;}function
isRelationLinks(){return
isset($this->params["relationLinks"])?$this->params["relationLinks"]:false;}function
getRecordsPerPage(){return(int)(isset($this->params["recordsPerPage"])?$this->params["recordsPerPage"]:50);}function
getEnumAsSelectThreshold(){if(array_key_exists("enumAsSelectThreshold",$this->params))return$this->params["enumAsSelectThreshold"]!==null?(int)$this->params["enumAsSelectThreshold"]:null;else
return
5;}function
isVersionVerificationEnabled(){return
isset($this->params["versionVerification"])?$this->params["versionVerification"]:true;}function
isSqlAutocompletionEnabled(){return
isset($this->params["sqlAutocompletion"])?$this->params["sqlAutocompletion"]:true;}function
getHiddenDatabases(){return$this->parseList(isset($this->params["hiddenDatabases"])?$this->params["hiddenDatabases"]:[]);}function
getHiddenSchemas(){return$this->parseList(isset($this->params["hiddenSchemas"])?$this->params["hiddenSchemas"]:[]);}function
getVisibleCollations(){return$this->parseList(isset($this->params["visibleCollations"])?$this->params["visibleCollations"]:[]);}function
getDefaultDriver(array$ed){$cd=isset($this->params["defaultDriver"])?$this->params["defaultDriver"]:null;return$cd&&isset($ed[$cd])?$cd:key($ed);}function
getDefaultServer(){$O=isset($this->params["defaultServer"])?$this->params["defaultServer"]:null;if($O===null)return
null;$Qk=isset($this->params["servers"][$O])?$this->params["servers"][$O]:null;if($Qk)return$Qk->getKey();return$O;}function
getDefaultDatabase(){return
isset($this->params["defaultDatabase"])?$this->params["defaultDatabase"]:null;}function
getDefaultPasswordHash(){return
isset($this->params["defaultPasswordHash"])?$this->params["defaultPasswordHash"]:null;}function
getSslKey(){return
isset($this->params["sslKey"])?$this->params["sslKey"]:null;}function
getSslCertificate(){return
isset($this->params["sslCertificate"])?$this->params["sslCertificate"]:null;}function
getSslCaCertificate(){return
isset($this->params["sslCaCertificate"])?$this->params["sslCaCertificate"]:null;}function
getSslTrustServerCertificate(){return
isset($this->params["sslTrustServerCertificate"])?$this->params["sslTrustServerCertificate"]:null;}function
getSslEncrypt(){return
isset($this->params["sslEncrypt"])?$this->params["sslEncrypt"]:null;}function
getSslMode(){return
isset($this->params["sslMode"])?$this->params["sslMode"]:null;}function
hasServers(){return
isset($this->params["servers"]);}function
getServerPairs(array$ed){$el=null;foreach($this->servers
as$O){if(!isset($ed[$O->getDriver()]))continue;if(!$el)$el=$O->getDriver();elseif($O->getDriver()!=$el){$el=null;break;}}$Rk=[];foreach($this->servers
as$u=>$O){if(!isset($ed[$O->getDriver()]))continue;$Pk=$O->getName();if($el&&$Pk)$Rk[$u]=$Pk;else$Rk[$u]=$ed[$O->getDriver()].($Pk!=""?" - $Pk":"");}return$Rk;}function
getServer($Ok){return
isset($this->servers[$Ok])?$this->servers[$Ok]:null;}function
applyServer($O){$O=$this->getServer($O);if(!$O)return;$this->params=array_merge($this->params,$O->getConfigParams());}private
function
parseList($Og){if(is_array($Og))return$Og;return
preg_split('~\s*,\s*~',(string)$Og);}}class
Settings{private
static$CookieName="neo_settings";static$ColorSchemeLight="light";static$ColorSchemeDark="dark";static$NavigationWidthMin=10;static$NavigationWidthMax=30;private$config;private$params=[];function
__construct(Config$Wb){$this->config=$Wb;if(isset($_COOKIE[self::$CookieName])){parse_str($_COOKIE[self::$CookieName],$this->params);$this->save();}if(isset($_COOKIE["neo_lang"])){$this->updateParameter("lang",$_COOKIE["neo_lang"]);unset($_COOKIE["neo_lang"]);cookie("neo_lang","",-3600);}}static
function
readParameter($u){parse_str(isset($_COOKIE[self::$CookieName])?$_COOKIE[self::$CookieName]:"",$Ti);return
isset($Ti[$u])?$Ti[$u]:null;}function
getParameter($u,$i=null){return
isset($this->params[$u])?$this->params[$u]:$i;}function
updateParameter($u,$Y){$this->updateParameters([$u=>$Y]);}function
updateParameters(array$Ti){$this->params=array_filter(array_merge($this->params,$Ti),function($Y){return$Y!==null;});$this->save();}private
function
save(){cookie(self::$CookieName,http_build_query($this->params),7776000);}function
getTheme(){return($ra=$this->getParameter("theme"))!==null?$ra:$this->config->getTheme();}function
getColorScheme(){return$this->getParameter("colorScheme");}function
getNavigationMode(){return($ra=$this->getParameter("navigationMode"))!==null?$ra:$this->config->getNavigationMode();}function
isNavigationSimple(){return$this->getNavigationMode()==Config::$NavigationSimple;}function
isNavigationDual(){return$this->getNavigationMode()==Config::$NavigationDual;}function
isNavigationHover(){return$this->getNavigationMode()==Config::$NavigationHover;}function
isNavigationReversed(){return$this->getNavigationMode()==Config::$NavigationReversed;}function
getNavigationWidth(){$Hn=$this->getParameter("navigationWidth");if($Hn===null)return
null;return
min(max((float)$Hn,self::$NavigationWidthMin),self::$NavigationWidthMax);}function
isSelectionPreferred(){return($ra=$this->getParameter("preferSelection"))!==null?$ra:$this->config->isSelectionPreferred();}function
isRelationLinks(){return
isset($this->params["relationLinks"])?$this->params["relationLinks"]:$this->config->isRelationLinks();}function
getRecordsPerPage(){return($ra=$this->getParameter("recordsPerPage"))!==null?$ra:$this->config->getRecordsPerPage();}function
getEnumAsSelectThreshold(){$Y=$this->getParameter("enumAsSelectThreshold");if($Y<0)return
null;return$Y!==null?(int)$Y:$this->config->getEnumAsSelectThreshold();}}class
Hash{static
function
hkdf($v,$u,$_f="",$tk=""){if(extension_loaded("hash")&&PHP_VERSION_ID>=70120)return
hash_hkdf("sha1",$u,$v,$_f,$tk);if($tk=="")$tk=str_repeat("\0",20);$Fj=self::hmacSha1($u,$tk);$hi="";for($lg="",$db=1;!isset($hi[$v-1]);$db++){$lg=self::hmacSha1($lg.$_f.chr($db),$Fj);$hi
.=$lg;}return
substr($hi,0,$v);}static
function
hmacSha1($f,$u){if(!extension_loaded("hash"))return
hash_hmac("sha1",$f,$u,true);if(strlen($u)>64)$u=sha1($u,true);$u=str_pad($u,64,"\0");$Qf=($u^str_repeat("\x36",64));$ri=($u^str_repeat("\x5C",64));return
sha1($ri.sha1($Qf.$f,true),true);}}class
Random{static
function
strongKey(){return
strtr(rtrim(base64_encode(Random::bytes(32)),"="),"+/","-_");}static
function
bytes($v){if(PHP_VERSION_ID>=70000)return
random_bytes($v);$J=self::tryAlternatives($v);if($J!==false)return$J;$J=self::lastResortRandom($v);if($J!==false)return$J;throw
new
Exception("Error generating random bytes");}private
static
function
tryAlternatives($v){if(extension_loaded("libsodium"))return
\Sodium\randombytes_buf($v);$Sm=DIRECTORY_SEPARATOR==="/";if($Sm){$J=self::readDevUrandom($v);if($J!==false)return$J;}$hb=$Sm&&PHP_VERSION_ID>50609&&PHP_VERSION_ID<50613;if(extension_loaded("mcrypt")&&!$hb){$J=mcrypt_create_iv($v,MCRYPT_DEV_URANDOM);if($J!==false)return$J;}$ib=PHP_VERSION_ID<50444||(PHP_VERSION_ID>50500&&PHP_VERSION_ID<50528)||(PHP_VERSION_ID>50600&&PHP_VERSION_ID<50612);if(extension_loaded("openssl")&&!$ib){$J=openssl_random_pseudo_bytes($v,$_l);if($_l)return$J;}return
false;}private
static
function
readDevUrandom($v){static$m=null;if($m===null)$m=@fopen("/dev/urandom","rb");if(!$m)return
false;$dk=$v;$J="";do{$f=fread($m,$dk);if($f===false)return
false;$dk-=strlen($f);$J
.=$f;}while($dk>0);return$J;}private
static
function
readCapicom($v){$Lb=new
\COM("CAPICOM.Utilities.1");$dk=$v;$J="";do{$f=base64_decode((string)$Lb->GetRandom($v,0));$dk-=strlen($f);$J
.=$f;}while($dk>0);return$J;}private
static
function
lastResortRandom($v){static$u=null;static$tk=null;if($u===null){$f=$_SERVER;$f[]=uniqid("",true);shuffle($f);$u=sha1(serialize($f),true);if(extension_loaded("openssl"))$tk=openssl_random_pseudo_bytes(20);else{$tk="";for($q=0;$q<20;$q++)$tk
.=chr((mt_rand()^mt_rand())%256);}}else{if((ord($u)%2===0)===(ord($tk)%2===0))$u=Hash::hmacSha1($u,$tk);else$tk=Hash::hmacSha1($tk,$u);}return
Hash::hkdf($v,$u,"$v",$tk);}}if(!function_exists("str_starts_with")){function
str_starts_with($Ye,$Nh){return
strpos($Ye,$Nh)===0;}}if(!function_exists("str_contains")){function
str_contains($Ye,$Nh){return
strpos($Ye,$Nh)!==false;}}if(!function_exists("password_verify")){function
password_verify($F,$Xe){return
false;}}if(!function_exists("ini_set")){function
ini_set($xi,$Y){return
false;}}function
version(){return
VERSION;}function
idf_unescape($qf){if(!preg_match('~^[`\'"[]~',$qf))return$qf;$xg=substr($qf,-1);$K=str_replace($xg.$xg,$xg,substr($qf,1,-1));return(DIALECT=="clickhouse"?str_replace("\\\\","\\",$K):$K);}function
q($yl){return
Connection::get()->quote($yl);}function
number($X){return
preg_replace('~[^0-9]+~','',$X);}function
int_type(){return'(tiny|small|medium|big)?int(eger|\d)?';}function
number_type(){return'(^('.int_type().'|decimal|numeric|real|(binary_|half_|scaled_)?float\d?|(binary_)?double( precision)?|(small)?money)$)';}function
remove_slashes(array$nn,$ie=false){$K=[];foreach($nn
as$u=>$X)$K[stripslashes($u)]=(is_array($X)?remove_slashes($X,$ie):($ie?$X:stripslashes($X)));return$K;}function
bracket_escape($qf,$Va=false){static$Bm=[':'=>':1',']'=>':2','['=>':3','"'=>':4'];return
strtr($qf,($Va?array_flip($Bm):$Bm));}function
min_version($qn,$Xg=null,$d=null){if(!$d)$d=Connection::get();if($Xg&&$d->isMariaDB())$qn=$Xg;return$qn&&$d->isMinVersion($qn);}function
charset(Connection$d){return($d->isMinVersion("5.5.3")?"utf8mb4":"utf8");}function
link_files($A,array$he){switch($A){case'favicon-blue.ico':$n='favicon-blue-0f5ce53a66b1e25395d0048da369f19e__32847818.ico';break;case'favicon-blue.svg':$n='favicon-blue-17e440832c1eac07527560a0d6f0d2ee__32847818.svg';break;case'apple-touch-icon-blue.png':$n='apple-touch-icon-blue-f2a5f6f50418d7293b806faf273fe381__32847818.png';break;case'logo.svg':$n='logo-de272eb4bdca9c6fffd38c073270fb1a__9d7e398f.svg';break;case'jush.css':$n='jush-b3a93b18444da26820ff61746521dede__9e257fe9.css';break;case'jush-dark.css':$n='jush-dark-f8dac59c6ad1018686e52a0e0357e421__2ec7793c.css';break;case'jush.js':$n='jush-615bc0b9720a1de8edd2c6876a3495b6__0e2d7929.js';break;case'icons.svg':$n='icons-70163a2695280bf75edba563e7b5471b__32847818.svg';break;case'default-blue.css':$n='default-blue-e7acfdb81453b86f081569afa115859e__cefac2cb.css';break;case'default-blue-dark.css':$n='default-blue-dark-1061ad7d216f143e3626560b92b66061__9e257fe9.css';break;case'main.js':$n='main-c1727fb392fab162b6611efe18642a5a__f59bfcb7.js';break;default:$n=null;break;}if(!$n)return
null;return
BASE_URL."?file=".urldecode($n);}function
ini_bool($xi){$X=ini_get($xi);return
preg_match('~^(on|true|yes)$~i',$X)||(int)$X;}function
ini_bytes($Bf){$X=ini_get($Bf);switch(strtolower(substr($X,-1))){case'g':$X=(int)$X*1024;case'm':$X=(int)$X*1024;case'k':$X=(int)$X*1024;}return$X;}function
max_input_vars($L,$Mi){$Zg=(int)ini_get("max_input_vars");return($Zg?(int)floor(($Zg-$Mi)/$L):0);}function
max_input_vars_error(){$Bf="max_input_vars";return
lang(103,"$Bf = ".(int)ini_get($Bf));}function
sid(){static$K;if($K===null)$K=(session_id()&&!($_COOKIE&&ini_bool("session.use_cookies")));return$K;}function
save_driver_name($cd,$O,$A){restart_session();$_SESSION["drivers"][$cd][$O]=$A;stop_session();}function
get_driver_name($cd,$O=null){return
isset($_SESSION["drivers"][$cd][$O])?$_SESSION["drivers"][$cd][$O]:Drivers::get($cd);}function
save_login($cd,$O,$V,$F,$h=""){$u=isset($_COOKIE["neo_key"])?$_COOKIE["neo_key"]:null;$_SESSION["pwds"][$cd][$O][$V]=$u?[encrypt_string($F,$u)]:$F;$_SESSION["db"][$cd][$O][$V][$h]=true;}function
delete_login($cd,$O,$V){unset($_SESSION["pwds"][$cd][$O][$V]);unset($_SESSION["db"][$cd][$O][$V]);unset($_SESSION["permanent"][$cd][$O][$V]);}function
get_password(){$F=get_session("pwds");if(is_array($F))return$_COOKIE["neo_key"]?decrypt_string($F[0],$_COOKIE["neo_key"]):false;return$F;}function
get_vals($I,$b=0){$K=[];$J=Connection::get()->query($I);if(is_object($J)){while($L=$J->fetchRow())$K[]=$L[$b];}return$K;}function
get_key_vals($I,$d=null,$Zk=true){if(!$d)$d=Connection::get();$K=[];$J=$d->query($I);if(is_object($J)){while($L=$J->fetchRow()){if($Zk)$K[$L[0]]=$L[1];else$K[]=$L[0];}}return$K;}function
get_rows($I,$d=null,$j="<p class='error'>"){if(!$d)$d=Connection::get();$K=[];$J=$d->query($I);if(is_object($J)){while($L=$J->fetchAssoc())$K[]=$L;}elseif(!$J&&!is_object($d)&&$j&&(defined("AdminNeo\PAGE_HEADER")||$j=="-- "))echo$j.error()."\n";return$K;}function
unique_array(array$L,array$t){foreach($t
as$s){if(!preg_match("~^(PRIMARY|UNIQUE)$~",$s["type"])||(isset($s["partial"])?$s["partial"]:"")!="")continue;$Pm=[];foreach($s["columns"]as$u){if(!isset($L[$u]))continue
2;$Pm[$u]=$L[$u];}return$Pm;}return
null;}function
escape_key($u){if(preg_match('(^([\w(]+)('.str_replace("_",".*",preg_quote(idf_escape("_"))).')([ \w)]+)$)',$u,$z))return$z[1].idf_escape(idf_unescape($z[2])).$z[3];return
idf_escape($u);}function
where($Z,$l=[]){$Vb=[];foreach((array)$Z["where"]as$u=>$X){$u=bracket_escape($u,true);$b=escape_key($u);$k=isset($l[$u])?$l[$u]:null;$de=isset($k["type"])?$k["type"]:null;$Ce=isset($k["full_type"])?$k["full_type"]:null;$Vf=$k&&(is_blob($k)||preg_match('~binary~',$de));if($Vf&&!is_utf8($X))$Vb[]="$b = ".Driver::get()->quoteBinary($X);elseif(DIALECT=="sql"&&$de=="json")$Vb[]="$b = CAST(".q($X)." AS JSON)";elseif(DIALECT=="pgsql"&&preg_match('~^jsonb?$~',$Ce))$Vb[]="$b::jsonb = ".q($X)."::jsonb";elseif(DIALECT=="sql"&&is_numeric($X)&&strpos($X,".")!==false)$Vb[]="$b LIKE ".q($X);elseif(DIALECT=="mssql"&&strpos($de,"datetime")===false)$Vb[]="$b LIKE ".q(preg_replace('~[_%[]~','[\0]',$X));else$Vb[]="$b = ".(isset($l[$u])?unconvert_field($l[$u],q($X)):q($X));if(DIALECT=="sql"&&preg_match('~char|text~',$de)&&preg_match("~[^ -@]~",$X))$Vb[]="$b = ".q($X)." COLLATE ".charset(Connection::get())."_bin";}foreach((array)$Z["null"]as$u)$Vb[]=escape_key($u)." IS NULL";return
implode(" AND ",$Vb);}function
where_columns($Z,$l=[]){$c=[];foreach((array)$Z["null"]as$u)$c[$u]=true;foreach((array)$Z["where"]as$u=>$X){$u=bracket_escape($u,true);foreach($l
as$A=>$k){if($u==$A||strpos($u,idf_escape($A))!==false)$c[$A]=true;}}return$c;}function
where_check($X,$l=[]){parse_str($X,$tb);remove_slashes([&$tb]);return
where($tb,$l);}function
validate_search_conditions($Vb,array$Pg=[]){$K=[];foreach((is_array($Vb)?$Vb:[])as$u=>$Ub){if(!is_array($Ub))continue;$Cb=isset($Ub["col"])?$Ub["col"]:"";$qi=isset($Ub["op"])?$Ub["op"]:"";$X=isset($Ub["val"])?$Ub["val"]:"";if(!is_string($Cb)||!is_string($qi))continue;$Yf=is_array($X)&&in_array($Cb,array_map('strval',$Pg),true)&&array_filter($X,'is_string')===$X;if(is_string($X)||$Yf)$K[$u]=$Ub;}return$K;}function
parse_search_list($Og){if(preg_match('~^\s*\((.*)\)\s*$~s',$Og,$z))$Og=$z[1];preg_match_all("~\\G\\s*+(?:'((?:''|[^'])*+)'\\s*+|([^,]*+))(?:,|\$)~s",$Og,$_,PREG_SET_ORDER);$K=[];foreach($_
as$z){if(!isset($z[2]))$K[]=str_replace("''","'",$z[1]);elseif(trim($z[2])!="")$K[]=trim($z[2]);}return$K;}function
where_link($q,$b,$Y,$ui="="){return"&where%5B$q%5D%5Bcol%5D=".urlencode($b)."&where%5B$q%5D%5Bop%5D=".urlencode(($Y!==null?$ui:"IS NULL"))."&where%5B$q%5D%5Bval%5D=".urlencode($Y);}function
convert_fields(array$c,array$l,array$N=[]){$J="";foreach($c
as$u=>$X){if($N&&!in_array(idf_escape($u),$N))continue;$Ma=convert_field($l[$u]);if($Ma)$J
.=", $Ma AS ".idf_escape($u);}return$J;}function
cookie_path(){return
strtr(preg_replace('~\?.*~','',$_SERVER["REQUEST_URI"]),[";"=>"%3B",","=>"%2C"]);}function
cookie($A,$Y,$Gg=2592000){header("Set-Cookie: $A=".rawurlencode($Y).($Gg?"; expires=".gmdate("D, d M Y H:i:s",time()+$Gg)." GMT":"")."; path=".cookie_path().(HTTPS?"; secure":"")."; HttpOnly; SameSite=lax",false);}function
get_url($an,$cc){$http_response_header=null;$Dd=[];set_error_handler(function($Bd,$j)use(&$Dd){$Dd[]=preg_replace('~^file_get_contents\([^)]*\):\s*~','',$j);return
true;});$K=file_get_contents($an,false,$cc);restore_error_handler();$af=function_exists('http_get_last_response_headers')?http_get_last_response_headers():$http_response_header;return[$K,(array)$af,$K===false?implode("\n",$Dd):"",];}function
get_settings($fc="neo_settings"){parse_str(isset($_COOKIE[$fc])?$_COOKIE[$fc]:"",$P);return$P;}function
get_setting($u,$fc="neo_settings"){$P=get_settings($fc);return
isset($P[$u])?$P[$u]:null;}function
save_settings(array$P,$fc="neo_settings"){cookie($fc,http_build_query($P+get_settings($fc)));}function
restart_session(){if(!ini_bool("session.use_cookies")&&session_status()==PHP_SESSION_NONE)session_start();}function
stop_session($qe=false){$en=ini_bool("session.use_cookies");if(!$en||$qe){session_write_close();if($en&&ini_set("session.use_cookies","0")===false)session_start();}}function&get_session($u){return$_SESSION[$u][DRIVER][SERVER][$_GET["username"]];}function
set_session($u,$X){$_SESSION[$u][DRIVER][SERVER][$_GET["username"]]=$X;}function
auth_url($pn,$O,$V,$h=null){$Zm=remove_from_uri(implode("|",array_keys(Drivers::getList()))."|username|ext|".($h!==null?"db|":"").($pn=='mssql'||$pn=='pgsql'?"":"ns|").session_name());preg_match('~([^?]*)\??(.*)~',$Zm,$z);return"$z[1]?".(sid()?session_name()."=".urlencode(session_id())."&":"").($_GET["ext"]?"ext=".urlencode($_GET["ext"])."&":"").urlencode($pn)."=".urlencode($O)."&"."username=".urlencode($V).($h!=""?"&db=".urlencode($h):"").($z[2]?"&$z[2]":"");}function
is_ajax(){return($_SERVER["HTTP_X_REQUESTED_WITH"]=="XMLHttpRequest");}function
redirect($y,$nh=null){if($nh!==null){restart_session();$_SESSION["messages"][preg_replace('~^[^?]*~','',($y!==null?$y:$_SERVER["REQUEST_URI"]))][]=$nh;}if($y!==null){if($y=="")$y=".";header("Location: $y");exit;}}function
query_redirect($I,$y,$nh,$Uj=true,$Jd=true,$Vd=false,$qm=""){if($Jd){$tl=microtime(true);$Vd=!Connection::get()->query($I);$qm=format_time($tl);}$nl=$I?Admin::get()->formatMessageQuery($I,$qm,$Vd):"";if($Vd){Admin::get()->addError(error().$nl.script("initToggles();"));return
false;}if($Uj)redirect($y,$nh.$nl);return
true;}function
queries_redirect($y,$nh,$Uj){$Kj=implode("\n",Queries::$queries);$qm=format_time(Queries::$start);return
query_redirect($Kj,$y,$nh,$Uj,false,!$Uj,$qm);}class
Queries{static$queries=[];static$start=0.0;}function
remember_query($I){if(!Queries::$start)Queries::$start=microtime(true);if(support("sql"))Queries::$queries[]=(preg_match('~;$~',$I)?"DELIMITER ;;\n$I;\nDELIMITER ":$I).";";else
Queries::$queries[]=$I;}function
queries($I){remember_query($I);return
support("sql")?Connection::get()->query($I):[];}function
apply_queries($I,array$S,$Ed='AdminNeo\table'){foreach($S
as$Q){if(!queries("$I ".$Ed($Q)))return
false;}return
true;}function
format_time($tl){return
lang(104,max(0,microtime(true)-$tl));}function
relative_uri(){return
str_replace(":","%3a",preg_replace('~^[^?]*/([^?]*)~','\1',$_SERVER["REQUEST_URI"]));}function
remove_from_uri($Si=""){return
substr(preg_replace("~(?<=[?&])($Si".(sid()?"":"|".session_name()).")=[^&]*&~",'',relative_uri()."&"),0,-1);}function
get_file($u,$Ac=false,$Hc=""){$m=$_FILES[$u];if(!$m)return
null;foreach($m
as$u=>$X)$m[$u]=(array)$X;$K='';foreach($m["error"]as$u=>$j){if($j)return$j;$A=$m["name"][$u];$xm=$m["tmp_name"][$u];$ac=file_get_contents($Ac&&preg_match('~\.gz$~',$A)?"compress.zlib://$xm":$xm);if($Ac){$tl=substr($ac,0,3);if(function_exists("iconv")&&preg_match("~^\xFE\xFF|^\xFF\xFE~",$tl))$ac=iconv("utf-16","utf-8",$ac);elseif($tl=="\xEF\xBB\xBF")$ac=substr($ac,3);}if($Hc){if(!preg_match("~$Hc\\s*\$~",$ac))$ac
.=";";$ac
.="\n\n";}$K
.=$ac;}return$K;}function
upload_error($j){$hh=($j==UPLOAD_ERR_INI_SIZE?ini_get("upload_max_filesize"):0);return($j?lang(105).($hh?" ".lang(106,$hh):""):lang(107));}function
repeat_pattern($ij,$v){return
str_repeat("$ij{0,65535}",$v/65535)."$ij{0,".($v%65535)."}";}function
is_utf8($X){return(preg_match('~~u',$X)&&!preg_match('~[\0-\x8\xB\xC\xE-\x1F]~',$X));}function
format_number($X){return
strtr(number_format($X,0,".",lang(108)),preg_split('~~u',lang(109),-1,PREG_SPLIT_NO_EMPTY));}function
format_rows(array$R){$M=$R["Rows"];$Ja=($M&&(DIALECT=="sqlite"||(isset($R["Engine"])?$R["Engine"]:"")==(DIALECT=="pgsql"?"table":"InnoDB")));return($Ja?"~ ":"").format_number($M);}function
friendly_url($X){return
preg_replace('~\W~i','-',$X);}function
table_status1($Q,$Xd=false){$K=table_status($Q,$Xd);return($K?reset($K):["Name"=>$Q]);}function
column_foreign_keys($Q){$K=[];foreach(Admin::get()->getForeignKeys($Q)as$o){foreach($o["source"]as$X)$K[$X][]=$o;}return$K;}function
fields_from_edit(){$K=[];foreach((array)$_POST["field_keys"]as$u=>$X){if($X!=""){$X=bracket_escape($X);$_POST["function"][$X]=$_POST["field_funs"][$u];$_POST["fields"][$X]=$_POST["field_vals"][$u];}}foreach((array)$_POST["fields"]as$u=>$X){$A=bracket_escape($u,true);$K[$A]=["field"=>$A,"full_type"=>"varchar","type"=>"varchar","privileges"=>["insert"=>1,"update"=>1,"where"=>1,"order"=>1],"null"=>true,"auto_increment"=>($u==Driver::get()->primary),];}return$K;}function
dump_headers($of,$Eh=false){$of=friendly_url($of).date("-Ymd-His");$Rd=Admin::get()->sendDumpHeaders($of,$Eh);if($Rd===null)$Rd=Admin::get()->sendDumpFormatHeaders($of,$Eh);Admin::get()->sendDumpOutputHeaders($of,$Rd);$Oi=$_POST["output"];if($Oi!="text")header("Content-Disposition: attachment; filename=$of.$Rd".($Oi!="file"&&preg_match('~^[0-9a-z]+$~',$Oi)?".$Oi":""));session_write_close();if(!ob_get_level())ob_start(null,4096);ob_flush();flush();return$Rd;}function
dump_table_order(array$Lh,array$ak){$qg=array_flip($Lh);$Bi=[];$yn=[];$rc=false;$xn=function($A)use(&$xn,&$Bi,&$yn,&$rc,$qg,$ak){if(isset($Bi[$A]))return;if(isset($yn[$A])){$rc=true;return;}$yn[$A]=true;foreach(isset($ak[$A])?$ak[$A]:[]as$Yj){if(isset($qg[$Yj]))$xn($Yj);}unset($yn[$A]);$Bi[$A]=true;};foreach($Lh
as$A)$xn($A);return($rc?null:array_keys($Bi));}function
dump_csv($L){$Im=$_POST["format"]=="tsv";foreach($L
as$u=>$X){if(preg_match('~["\n]|^0[^.]|\.\d*0$|'.($Im?'\t':'[,;]|^$').'~',$X))$L[$u]='"'.str_replace('"','""',$X).'"';}echo
implode(($_POST["format"]=="csv"?",":($Im?"\t":";")),$L)."\r\n";}function
apply_sql_function($p,$b){return($p?($p=="unixepoch"?"DATETIME($b, '$p')":($p=="count distinct"?"COUNT(DISTINCT ":strtoupper("$p("))."$b)"):$b);}function
get_temp_dir(){$gj=ini_get("upload_tmp_dir");if(!$gj)$gj=sys_get_temp_dir();return$gj;}function
open_file_with_lock($n){if(is_link($n))return
null;$m=@fopen($n,"c+");if(!$m)return
null;@chmod($n,0660);if(!flock($m,LOCK_EX)){fclose($m);return
null;}return$m;}function
write_and_unlock_file($m,$f){rewind($m);fwrite($m,$f);ftruncate($m,strlen($f));unlock_file($m);}function
unlock_file($m){flock($m,LOCK_UN);fclose($m);}function
first(array$La){return
reset($La);}function
get_private_key($e){$n=get_temp_dir()."/adminneo.key";if(!$e&&!file_exists($n))return
false;$m=open_file_with_lock($n);if(!$m)return
false;$u=stream_get_contents($m);if(!$u){$u=Random::strongKey();write_and_unlock_file($m,$u);}else
unlock_file($m);return$u;}function
get_random_string(){return
Random::strongKey();}function
select_value($X,$x,$k,$mm){if(is_array($X)){$K="";if(array_filter($X,'is_array')==array_values($X)){$mg=[];foreach($X
as$W)$mg+=array_fill_keys(array_keys($W),null);foreach(array_keys($mg)as$hg)$K
.="<th>".h($hg);foreach($X
as$W){$K
.="<tr>";foreach(array_merge($mg,$W)as$kn)$K
.="<td>".select_value($kn,$x,$k,$mm);}}else{foreach($X
as$hg=>$W)$K
.="<tr>".($X!=array_values($X)?"<th>".h($hg):"")."<td>".select_value($W,$x,$k,$mm);}return"<table>$K</table>";}$yk="";if($k&&$X!==null&&($mm===null||strlen($X)<=$mm)&&($nn=Driver::get()->explodeArrayValue($X,$k["full_type"],$yk))){$xk=$k;$xk["type"]=$xk["full_type"]=$yk;$K=select_array_value($nn,$X,$x,$xk,$mm);return
Driver::get()->implodeArrayValues($K,$k["full_type"]);}if(!$x)$x=Admin::get()->getFieldValueLink($X,$k);if($k)$X=Connection::get()->formatValue($X,$k);$K=$k?Admin::get()->formatFieldValue($X,$k):$X;if($K!==null){if(!is_utf8($K))$K="\0";elseif($mm!=""&&is_shortable($k))$K=truncate_utf8($K,max(0,+$mm));else$K=h($K);}return
Admin::get()->formatSelectionValue($K,$x,$k,$X);}function
select_array_value(array$nn,$X,$x,array$k,$mm){$J=[];foreach($nn
as$Y){if(is_array($Y))$J[]=select_array_value($Y,$X,$x,$k,$mm);else{$rg=preg_replace('~(where%5B\d+%5D%5Bval%5D=)'.preg_quote(urlencode($X),"~")."~",'${1}'.urlencode($Y),$x);$J[]=select_value($Y,$rg,$k,$mm);}}return$J;}function
is_blob(array$k){$Mm=Driver::get()->getStructuredTypes();$U=lang(110);return
preg_match('~blob|bytea|raw|file'.(DIALECT=="mssql"?'|binary|image':'').'~',$k["type"])&&!in_array($k["type"],isset($Mm[$U])?$Mm[$U]:[]);}function
is_generated_always(array$k){return(isset($k["generated"])?$k["generated"]:"")!=""||stripos((string)(isset($k["default"])?$k["default"]:""),"GENERATED ALWAYS AS ")===0;}function
parenthesize_expression($Pd){$zl=preg_replace('~\'(?:[^\']|\'\')*\'|"(?:[^"]|"")*"|`(?:[^`]|``)*`|\[[^]]*]~',"",$Pd);if(!preg_match('~^\(.*\)$~s',$zl))return"($Pd)";$Jc=0;$xg=strlen($zl)-1;for($q=0;$q<$xg;$q++){if($zl[$q]=="(")$Jc++;elseif($zl[$q]==")"&&--$Jc==0)return"($Pd)";}return$Pd;}function
is_mail($Y){return
is_string($Y)&&filter_var($Y,FILTER_VALIDATE_EMAIL);}function
is_web_url($Y){if(!is_string($Y)||!preg_match('~^(https?:)?//~i',$Y))return
false;$Sb=parse_url($Y);if(!$Sb)return
false;$an=$Y;if(isset($Sb['path'])){$wd=array_map('urlencode',explode('/',$Sb['path']));$an=str_replace($Sb['path'],implode('/',$wd),$an);}if(isset($Sb['query'])){parse_str($Sb['query'],$Ti);$an=str_replace($Sb['query'],http_build_query($Ti),$an);}if(!isset($Sb['scheme']))$an="https:$an";return(bool)filter_var($an,FILTER_VALIDATE_URL);}function
is_shortable($k){return$k&&!preg_match('~'.number_type().'|date|time|year~',$k["type"]);}function
host_port($O){return(preg_match('~^(:([^:].*)|(\[(.+)]|(([^:]+://)?[^:]+))(:(\d+))?)$~',$O,$z)?[(isset($z[4])?$z[4]:"").(isset($z[5])?$z[5]:""),$z[2].(isset($z[8])?$z[8]:"")]:[$O,'']);}function
is_host_valid($jf){if(preg_match('~^\[(.+)]$~D',$jf,$z))return(bool)filter_var($z[1],FILTER_VALIDATE_IP,FILTER_FLAG_IPV6);return(bool)preg_match('~^[-\w.]+$~Du',$jf);}function
count_rows($Q,$Z,$Wf,$Ne){$I=" FROM ".table($Q).($Z?" WHERE ".implode(" AND ",$Z):"");return($Wf&&(DIALECT=="sql"||count($Ne)==1)?"SELECT COUNT(DISTINCT ".implode(", ",$Ne).")$I":"SELECT COUNT(*)".($Wf?" FROM (SELECT 1$I GROUP BY ".implode(", ",$Ne).") x":$I));}function
slow_query($I){$h=Admin::get()->getDatabase();$rm=Admin::get()->getQueryTimeout();$gl=Driver::get()->slowQuery($I,$rm);$d=null;if(!$gl&&support("kill")){$d=connect();if($d&&($h==""||$d->selectDatabase($h))){$og=number($d->getValue(connection_id()));echo'<script',nonce(),'>
	const timeout = setTimeout(() => {
		ajax(\'',js_escape(ME),'script=kill\', function() {
		}, \'kill=',$og,'&token=',get_token(),'\');
	}, ',1000*$rm,');
</script>
';}}ob_flush();flush();$K=@get_key_vals(($gl?:$I),$d,false);if($d){echo
script("clearTimeout(timeout);");ob_flush();flush();}return$K;}function
get_token(){$Pj=rand(1,1e6);return($Pj^$_SESSION["token"]).":$Pj";}function
verify_token(){list($ym,$Pj)=explode(":",$_POST["token"]);return($Pj^$_SESSION["token"])==$ym&&in_array($_SERVER["HTTP_SEC_FETCH_SITE"],["","same-origin"]);}function
is_same_origin(){if(isset($_SERVER["HTTP_SEC_FETCH_SITE"]))return$_SERVER["HTTP_SEC_FETCH_SITE"]=="same-origin";return
isset($_SERVER["HTTP_REFERER"])&&is_same_host($_SERVER["HTTP_REFERER"]);}function
is_same_host($an){return
isset($_SERVER["HTTP_HOST"])&&preg_match('~^https?://([^/?#]*)~i',$an,$z)&&!strcasecmp($z[1],$_SERVER["HTTP_HOST"]);}function
is_authenticated($Pa=false){static$J=false;if($Pa)$J=true;return$J;}function
script($kl,$Am="\n"){return"<script".nonce().">$kl</script>$Am";}function
script_src($an,$Ec=false){return"<script src='".h($an)."'".nonce().($Ec?" defer":"")."></script>\n";}function
nonce(){return' nonce="'.get_nonce().'"';}function
input_hidden($A,$Y=""){return"<input type='hidden' name='".h($A)."' value='".h($Y)."'>";}function
input_token(){return
input_hidden("token",get_token());}function
target_blank(){return' target="_blank" rel="noreferrer noopener"';}function
h($yl){if($yl===null||$yl==="")return"";return
str_replace(["&","<","\"","'","\0"],["&amp;","&lt;","&quot;","&#039;","&#0;"],$yl);}function
truncate_utf8($yl,$v=80){if($yl=="")return"";if(!preg_match("(^(".repeat_pattern("[\t\r\n -\x{10FFFF}]",$v).")($)?)u",$yl,$z))preg_match("(^(".repeat_pattern("[\t\r\n -~]",$v).")($)?)",$yl,$z);return
h($z[1]).(isset($z[2])?"":"<i>…</i>");}function
icon_solo($r){return
icon($r,"solo");}function
icon_chevron_down(){return
icon("chevron-down","chevron");}function
icon_chevron_right(){return
icon("chevron-down","chevron-right");}function
icon($r,$Ab=null){$r=h($r);return"<svg class='icon ic-$r $Ab'><use href='".h(link_files("icons.svg",[]))."#$r'/></svg>";}function
checkbox($A,$Y,$wb,$sg="",$pi="",$Ab="",$ug=""){$K="<input type='checkbox' name='".h($A)."' value='".h($Y)."'".($wb?" checked":"").($ug?" aria-labelledby='$ug'":"").">".($pi?script("qsl('input').onclick = function () { $pi };",""):"");return($sg!=""||$Ab?"<label".($Ab?" class='$Ab'":"").">$K".h($sg)."</label>":$K);}function
optionlist($C,$Ik=null,$fn=false){$K="";foreach($C
as$hg=>$W){$zi=[$hg=>$W];if(is_array($W)){$K
.='<optgroup label="'.h($hg).'">';$zi=$W;}foreach($zi
as$u=>$X)$K
.='<option'.($fn||is_string($u)?' value="'.h($u).'"':'').($Ik!==null&&($fn||is_string($u)?(string)$u:$X)===$Ik?' selected':'').'>'.h($X);if(is_array($W))$K
.='</optgroup>';}return$K;}function
html_select($A,$C,$Y="",$oi="",$ug="",$fn=false){static$sg=0;$tg="";if(!$ug&&substr(isset($C[""])?$C[""]:"",0,1)=="("){$sg++;$ug="label-$sg";$tg="<option value='' id='$ug'>".h($C[""]);unset($C[""]);}return"<select name='".h($A)."'".($ug?" aria-labelledby='$ug'":"").">".$tg.optionlist($C,$Y,$fn)."</select>".($oi?script("qsl('select').onchange = function () { $oi };",""):"");}function
html_radios($A,$C,$Y=""){$J="<span class='labels'>";foreach($C
as$u=>$X)$J
.="<label><input type='radio' name='".h($A)."' value='".h($u)."'".($u==$Y?" checked":"").">".h($X)."</label>";$J
.="</span>";return$J;}function
confirm($nh="",$Kk="qsl('input')"){return
script("$Kk.onclick = () => confirm('".js_escape($nh?:lang(111))."');","");}function
print_fieldset_start($r,$Cg,$nf,$vn=false,$il=false){echo"<fieldset id='fieldset-$r' class='closable ".(!$vn?" closed":"")."'>","<legend><a href='#'>$Cg</a></legend>",icon($nf,"fieldset-icon jsonly"),"<div class='fieldset-content".($il?" sortable":"")."'>";}function
print_fieldset_end($r,$il=false){echo"</div>",script("initFieldset('$r');","");if($il)echo
script("initSortable('#fieldset-$r .fieldset-content');","");echo"</fieldset>\n";}function
bold($eb,$Ab=""){return($eb?" class='$Ab active'":($Ab?" class='$Ab'":""));}function
js_escape($yl){return
str_replace("<","\\x3C",addcslashes($yl,"\r\n'\\"));}function
js_escape_key($yl){return'"'.str_replace("<","\\x3C",addcslashes($yl,"\r\n\t\"\\")).'"';}function
js_escape_re($yl){return
addcslashes(preg_quote($yl,"/"),"\r\n");}function
pagination($E,$oc){return"<li>".($E==$oc?"<strong>".($E+1)."</strong>":'<a href="'.h(remove_from_uri("page").($E?"&page=$E".($_GET["next"]?"&next=".urlencode($_GET["next"]):""):"")).'">'.($E+1)."</a>")."</li>";}function
print_hidden_fields(array$Gj,array$rf=[],$yj=""){$J=false;foreach($Gj
as$u=>$X){if(!in_array($u,$rf)){if(is_array($X))print_hidden_fields($X,[],$u);else{$J=true;echo
input_hidden($yj?$yj."[$u]":$u,$X);}}}return$J;}function
hidden_fields_get(){if(sid())echo
input_hidden(session_name(),session_id());if($_GET["ext"])echo
input_hidden("ext",$_GET["ext"]);if(SERVER!==null)echo
input_hidden(DRIVER,SERVER);echo
input_hidden("username",$_GET["username"]);}function
enum_input($Na,array$k,$Y,$ud=null,$vb=false){preg_match_all("~'((?:[^']|'')*)'~",$k["length"],$_);$nn=$_[1];$pm=Admin::get()->getSettings()->getEnumAsSelectThreshold();$N=!$vb&&$pm!==null&&count($nn)>$pm;$U=$vb?"checkbox":"radio";$xa=$N?"selected":"checked";$J=$N?"<select $Na>":"<span class='labels'>";if($N&&$k["null"]&&$ud!==""){$wb=$Y===null?$xa:"";$J
.="<option value='__adminneo_empty__' disabled $wb></option>";}if($ud!==null){$wb=(is_array($Y)?in_array($ud,$Y):$Y===$ud)?$xa:"";if($N)$J
.="<option value='$ud' $wb>".lang(112)."</option>";else$J
.="<label><input type='$U' $Na value='$ud' $wb><i>".lang(112)."</i></label>";}foreach($nn
as$X){if($ud===""&&$X==="")continue;$X=stripcslashes(str_replace("''","'",$X));$wb=is_array($Y)?in_array($X,$Y):$Y===$X;$wb=$wb?$xa:"";$ye=$X===""?("<i>".lang(112)."</i>"):h(Admin::get()->formatFieldValue($X,$k));if($N)$J
.="<option value='".h($X)."' $wb>$ye</option>";else$J
.=" <label><input type='$U' $Na value='".h($X)."' $wb>$ye</label>";}$J
.=$N?"</select>":"</span>";return$J;}function
input($k,$Y,$p,$Sa=false){$A=h(bracket_escape($k["field"]));$Mm=Driver::get()->getTypes();$Xf=isset($k["full_type"])&&Admin::get()->detectJson($k["full_type"],$Y,true);$fk=(DIALECT=="mssql"&&$k["auto_increment"]&&!$_POST["clone"]);if($fk&&!$_POST["save"])$p=null;$Km=Driver::get()->getUserTypeId(isset($k["type"])?$k["type"]:"");if($Km){$Ad=type_values($Km);if($Ad){$k["type"]="enum";$k["length"]=$Ad;}}$Na=" name='fields[$A]' ".($Sa?" autofocus":"");$Fe=(isset($_GET["select"])||$fk?["orig"=>lang(113)]:[])+Admin::get()->getFieldFunctions($k);$We=(in_array($p,$Fe)||isset($Fe[$p]));echo"<td class='function'>",Driver::get()->getUnconvertFunction($k)." ";if(count($Fe)>1){$Ik=$p===null||$We?$p:"";echo"<select name='function[$A]'>".optionlist($Fe,$Ik)."</select>",help_script_command("value.replace(/^SQL\$/, '')",true),script("qsl('select').onchange = functionChange;","");}else
echo
h(reset($Fe));echo"</td><td>";$Cf=Admin::get()->getFieldInput(isset($_GET["edit"])?$_GET["edit"]:null,$k,$Na,$Y,$p);if($Cf!="")echo$Cf;elseif(preg_match('~bool~',$k["type"]))echo"<input type='hidden'$Na value='0'>"."<input type='checkbox'".(preg_match('~^(1|t|true|y|yes|on)$~i',$Y)?" checked":"")."$Na value='1'>";elseif($k["type"]=="enum")echo
enum_input($Na,$k,$Y);elseif($k["type"]=="set"){preg_match_all("~'((?:[^']|'')*)'~",$k["length"],$_);echo"<span class='labels'>";foreach($_[1]as$X){$X=stripcslashes(str_replace("''","'",$X));$wb=$Y!==null&&in_array($X,explode(",",$Y),true);$wb=$wb?"checked":"";$ye=$X===""?("<i>".lang(112)."</i>"):h(Admin::get()->formatFieldValue($X,$k));echo" <label><input type='checkbox' name='fields[$A][]' value='".h($X)."' $wb>$ye</label>";}echo"</span>";}elseif(is_blob($k)&&ini_bool("file_uploads"))echo"<input type='file' name='fields-$A'>";elseif($Xf)echo"<textarea $Na cols='50' rows='12' class='jush-json'>".h($Y).'</textarea>';elseif(($km=preg_match('~text|lob|memo|json~i',$k["type"])||(preg_match('~char|binary~',$k["type"])&&$k["length"]=="max"))||preg_match("~\n~",$Y)){if($km&&DIALECT!="sqlite")$Na
.=" cols='50' rows='12'";else{$M=min(12,substr_count($Y,"\n")+1);$Na
.=" cols='30' rows='$M'";}echo"<textarea $Na>".h($Y).'</textarea>';}else{$jh=!preg_match('~int~',$k["type"])&&preg_match('~^(\d+)(,(\d+))?$~',$k["length"],$z)?((preg_match("~binary~",$k["type"])?2:1)*$z[1]+($z[3]?1:0)+($z[2]&&!$k["unsigned"]?1:0)):($Mm&&$Mm[$k["type"]]?$Mm[$k["type"]]+($k["unsigned"]?0:1):0);if(DIALECT=='sql'&&Connection::get()->isMinVersion("5.6")&&preg_match('~time~',$k["type"]))$jh+=7;echo"<input class='input'".((!$We||$p==="")&&preg_match('~^'.int_type().'$~',$k["type"])&&!preg_match('~\[]~',$k["full_type"])?" type='number'":"").($p!="now"?" value='".h($Y)."'":" data-last-value='".h($Y)."'").($jh?" data-maxlength='$jh'":"").(preg_match('~char|binary~',$k["type"])&&$jh>20?" size='44'":"")."$Na>";}$ef=Admin::get()->getFieldInputHint($_GET["edit"],$k,$Y);if($ef!="")echo" <span class='input-hint'>$ef</span>";if(count($Fe)>1)echo
script("qs('select', qsl('td').previousSibling).onchange(null, true);","");$me=0;foreach($Fe
as$u=>$X){if($u===""||!$X)break;$me++;}if(count($Fe)>1)echo
script("qsl('td').oninput = partial(skipOriginal, $me);");}function
process_input($k){if(is_generated_always($k))return
null;$qf=bracket_escape($k["field"]);$p=isset($_POST["function"][$qf])?$_POST["function"][$qf]:"";if($p=="orig")return(preg_match('~^CURRENT_TIMESTAMP~i',$k["on_update"])?idf_escape($k["field"]):false);if($p=="NULL")return
Driver::get()->getNull();if(is_blob($k)&&ini_bool("file_uploads")){$m=get_file("fields-$qf");if(!is_string($m))return
false;return
Driver::get()->quoteBinary($m);}$Y=isset($_POST["fields"][$qf])?$_POST["fields"][$qf]:(isset($_FILES["fields"]["name"][$qf])?$_FILES["fields"]["name"][$qf]:null);if($Y===null)return
false;if($k["auto_increment"]&&$Y=="")return
null;if($k["type"]=="set")$Y=implode(",",(array)$Y);if($p=="json"){$Y=json_decode($Y,true);if(!is_array($Y))return
false;return$Y;}if($p!="SQL"&&isset($k["full_type"]))Admin::get()->detectJson($k["full_type"],$Y,false);return
Admin::get()->processFieldInput($k,$Y,$p);}function
search_tables(){$_GET["where"][0]["val"]=$_POST["query"];$kk=$Dd=[];foreach(table_status("",true)as$Q=>$R){$Ul=Admin::get()->getTableName($R);if(!isset($R["Engine"])||$Ul==""||($_POST["tables"]&&!in_array($Q,$_POST["tables"])))continue;$J=Connection::get()->query("SELECT".limit("1 FROM ".table($Q)," WHERE ".implode(" AND ",Admin::get()->processSelectionSearch(fields($Q),[])),1));if($J&&!$J->fetchRow())continue;$x=h(ME."select=".urlencode($Q)."&where[0][op]=".urlencode($_GET["where"][0]["op"])."&where[0][val]=".urlencode($_GET["where"][0]["val"]));if($J)$kk[]="<li><a href='$x'>".icon("search")."$Ul</a></li>";else$Dd[]="<div class='error'><a href='$x'>$Ul</a>: ".error()."</div>";}if($kk)echo"<ul class='links'>\n",implode("\n",$kk),"</ul>\n";if($Dd)echo
implode("\n",$Dd),"\n";if(!$kk&&!$Dd)echo"<p class='message'>".lang(79)."</p>\n";}function
help_script($km,$dl=false){return
script("initHelpFor(qsl('select, input'), '".js_escape($km)."', $dl);","");}function
help_script_command($Mb,$dl=false){return
script("initHelpFor(qsl('select, input'), (value) => { return $Mb; }, $dl);","");}function
edit_form($Q,$l,$L,$Ym){$Ul=Admin::get()->getTableName(table_status1($Q,true));$T=$Ym?lang(39):lang(114);page_header("$T: $Ul",["select"=>[$Q,$Ul],$T]);if($L===false){echo"<p class='error'>".lang(90)."\n";return;}echo"<form action='' method='post' enctype='multipart/form-data' id='form'>\n";$qd=false;$Gn=($Ym&&!isset($_GET["select"])?where_columns($_GET,$l):[]);$dc=(count($Gn)!=count($l));if(!$dc)$Gn=[];if(!$l)echo"<p class='error'>".lang(115)."\n";else{echo"<table class='box'>".script("qsl('table').onkeydown = onEditingKeydown;");$Sa=!$_POST;foreach($l
as$A=>$k){echo"<tr".(isset($Gn[$A])?" class='where-column'":"")."><th>".Admin::get()->getFieldName($k);$u=bracket_escape($A);$i=isset($_GET["preset"][$u])?$_GET["preset"][$u]:null;if($i===null){$i=$k["default"];if($k["type"]=="bit"&&preg_match("~^b'([01]*)'\$~",$i,$ck))$i=$ck[1];if(DIALECT=="sql"&&preg_match('~binary~',$k["type"]))$i=bin2hex($i);}$Y=($L!==null?($L[$A]!=""&&DIALECT=="sql"&&preg_match("~enum|set~",$k["type"])&&is_array($L[$A])?implode(",",$L[$A]):(is_bool($L[$A])?+$L[$A]:$L[$A])):(!$Ym&&$k["auto_increment"]?"":(isset($_GET["select"])?false:$i)));if(!$_POST["save"]&&is_string($Y))$Y=Admin::get()->formatFieldValue($Y,$k);if(($Ym&&!isset($k["privileges"]["update"]))||is_generated_always($k)){echo"<td class='function'></td><td>";if($Ym||!$k["generated"])echo
select_value($Y,'',$k,null);else
echo"<code class='jush-".DIALECT."'>",h($Y),"</code>";echo"</td>";}else{$qd=true;$p=($_POST["save"]?isset($_POST["function"][$u])?$_POST["function"][$u]:"":($Ym&&preg_match('~^CURRENT_TIMESTAMP~i',$k["on_update"])?"now":($Y===false?null:($Y!==null?'':'NULL'))));if(!$_POST&&!$Ym&&$Y==$k["default"]&&preg_match('~^[\w.]+\(~',$Y))$p="SQL";if(preg_match("~time~",$k["type"])&&preg_match('~^CURRENT_TIMESTAMP~i',$Y)){$Y="";$p="now";}if($k["type"]=="uuid"&&$Y=="uuid()"){$Y="";$p="uuid";}if($Sa!==false)$Sa=($k["auto_increment"]||$p=="now"||$p=="uuid"?null:true);input($k,$Y,$p,(bool)$Sa);if($Sa)$Sa=false;}echo"\n";}if(!support("table")&&!fields($Q))echo"<tr>"."<th><input class='input' name='field_keys[]'>".script("qsl('input').oninput = fieldChange;","")."<td class='function'>".html_select("field_funs[]",Admin::get()->getFieldFunctions(["null"=>isset($_GET["select"])]))."<td><input class='input' name='field_vals[]'>"."\n";echo"</table>\n",script("initToggles(gid('form'));");if($Gn)echo
script("initWhereChange();");}echo"<p>";if($qd){echo"<input type='submit' class='button default' value='".lang(116)."'>\n";if(!isset($_GET["select"])&&$dc){$Sc=($Gn&&Admin::get()->getErrors()?" disabled":"");echo"<input type='submit' class='button' name='insert' value='".($Ym?lang(117):lang(118))."' title='Ctrl+Shift+Enter'$Sc>\n",($Ym?script("qsl('input').onclick = function () { return !ajaxForm(this.form, '".js_escape(lang(119))."', this); };"):"");}}echo($Ym?"<input type='submit' class='button' name='delete' value='".lang(120)."'>".confirm()."\n":"");if(isset($_GET["select"]))print_hidden_fields(["check"=>(array)$_POST["check"],"clone"=>$_POST["clone"],"all"=>$_POST["all"]]);echo
input_hidden("referer",isset($_POST["referer"])?$_POST["referer"]:$_SERVER["HTTP_REFERER"]),input_hidden("save","1"),input_token(),"</form>\n";}function
file_upload_form_script($ue,$Df){$bh=ini_get("max_file_uploads");$hh=ini_get("upload_max_filesize");$ih=ini_bytes("upload_max_filesize");return
script("initFilesUploadForm('".js_escape($ue)."', '".js_escape($Df)."', "."$bh, '".js_escape(lang(121,$bh,"'max_file_uploads'"))."', "."$ih, '".js_escape(lang(122,$hh,"'upload_max_filesize'"))."')");}function
compress_alphabet(){return
strtr(implode(range('"','~')),"'\\","!\n");}function
decompress_string($yl,$Nc=""){$Ga=array_flip(str_split(compress_alphabet()));$v=strlen($yl);$mn=($v?13*($v-1)/2-$Ga[$yl[0]]:0);$ab="";$ik=0;$jk=0;for($q=1;$q<$v;$q+=2){$ik=($ik<<13)+$Ga[$yl[$q]]*93+$Ga[$yl[$q+1]];$jk+=13;while($jk>=8&&$mn>=8){$jk-=8;$mn-=8;$ab
.=chr($ik>>$jk);$ik&=(1<<$jk)-1;}}if($ab=="")return"";if($Nc!=""&&function_exists('inflate_init'))return
inflate_add(inflate_init(ZLIB_ENCODING_RAW,['dictionary'=>$Nc]),$ab,ZLIB_FINISH);return$Nc==""&&function_exists('gzinflate')?gzinflate($ab):inflate($ab,$Nc);}function
inflate($ab,$Nc=""){$Dg=[3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131,163,195,227,258];$Eg=[0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0];$Vc=[1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025,1537,2049,3073,4097,6145,8193,12289,16385,24577];$Xc=[0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13];$K=$Nc;$H=0;do{$ke=inflate_bits($ab,$H,1);$U=inflate_bits($ab,$H,2);if(!$U){$H=($H+7)&~7;$v=inflate_bits($ab,$H,16);$H+=16;$K
.=substr($ab,$H>>3,$v);$H+=$v<<3;}else{if($U==1){$Rg=array_merge(array_fill(0,144,8),array_fill(0,112,9),array_fill(0,24,7),array_fill(0,8,8));$Yc=array_fill(0,30,5);}else{$Qg=inflate_bits($ab,$H,5)+257;$Wc=inflate_bits($ab,$H,5)+1;$D=[16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15];$th=array_fill(0,19,0);$sh=inflate_bits($ab,$H,4)+4;for($q=0;$q<$sh;$q++)$th[$D[$q]]=inflate_bits($ab,$H,3);$uh=inflate_table($th);$Fg=[];while(count($Fg)<$Qg+$Wc){$Kl=inflate_symbol($ab,$H,$uh);if($Kl==16)$Fg=array_merge($Fg,array_fill(0,inflate_bits($ab,$H,2)+3,end($Fg)));elseif($Kl==17)$Fg=array_merge($Fg,array_fill(0,inflate_bits($ab,$H,3)+3,0));elseif($Kl==18)$Fg=array_merge($Fg,array_fill(0,inflate_bits($ab,$H,7)+11,0));else$Fg[]=$Kl;}$Rg=array_slice($Fg,0,$Qg);$Yc=array_slice($Fg,$Qg);}$Sg=inflate_table($Rg);$ad=inflate_table($Yc);while(($Kl=inflate_symbol($ab,$H,$Sg))!=256){if($Kl<256)$K
.=chr($Kl);else{$v=$Dg[$Kl-257]+inflate_bits($ab,$H,$Eg[$Kl-257]);$Zc=inflate_symbol($ab,$H,$ad);$di=strlen($K)-$Vc[$Zc]-inflate_bits($ab,$H,$Xc[$Zc]);for($q=0;$q<$v;$q++)$K
.=$K[$di+$q];}}}}while(!$ke);return$Nc==""?$K:substr($K,strlen($Nc));}function
inflate_bits($ab,&$H,$jc){$K=0;for($q=0;$q<$jc;$q++){$K+=((ord($ab[$H>>3])>>($H&7))&1)<<$q;$H++;}return$K;}function
inflate_table(array$Fg){$Q=[];$Bb=0;for($bb=1;$bb<=max($Fg);$bb++){foreach($Fg
as$Kl=>$v){if($v==$bb){$Q[$bb][$Bb]=$Kl;$Bb++;}}$Bb<<=1;}return$Q;}function
inflate_symbol($ab,&$H,array$Q){$Bb=0;$bb=0;do{$Bb=($Bb<<1)+inflate_bits($ab,$H,1);$bb++;}while(!isset($Q[$bb][$Bb]));return$Q[$bb][$Bb];}if(isset($_GET["file"]))load_compiled_file($_GET["file"]);function
load_compiled_file($n){if($n==""){http_response_code(404);exit;}if($_SERVER["HTTP_IF_MODIFIED_SINCE"]){http_response_code(304);exit;}header("Expires: ".gmdate("D, d M Y H:i:s",time()+365*24*60*60)." GMT");header("Last-Modified: ".gmdate("D, d M Y H:i:s")." GMT");header("Cache-Control: immutable");ini_set("zlib.output_compression","1");$Rd=pathinfo($n,PATHINFO_EXTENSION);switch($Rd){case"css":header("Content-Type: text/css; charset=utf-8");break;case"js":header("Content-Type: text/javascript; charset=utf-8");break;case"ico":header("Content-Type: image/x-icon");break;case"png":header("Content-Type: image/png");break;case"svg":header("Content-Type: image/svg+xml");break;}switch($n){case'favicon-blue-0f5ce53a66b1e25395d0048da369f19e__32847818.ico':$f='AAABAAEAICAAAAEAIAC6AQAAFgAAAIlQTkcNChoKAAAADUlIRFIAAAAgAAAAIAgGAAAAc3p69AAAAYFJREFUeNrV1wEEGmEYh/FztCYBRATANhCAAEGAEGZowEUFhM2G6A4QAJksoMi2AYRlAxgcAUgthAS2yTFo5d2DDzbO6r2PhB9APY73z+cUn3+6qbsJcFGCjxlCbPHL2CLEDD5KcG0EPESAH5ArfUeAtDbgCb5BElrjsSbgI8SSD5qAM8SSsyZAbNIErCGWrDQBTYglTe0ZNnCAKB3gJR2iAnwsIBdawEchyRC9jompoYUe3hg9tFCL+dNX2ivo4wEcpTT6EF0AsEMHeTgXyqODnf4M489phC7aeGq00cUIK1s7sLr1DryEWPJCE5DBBJLQBJkkO9DAHnKlPbwkO/AMjuGijCGWiCD/iLDEEGW4f/2WIuA3qnBiZPHIyMKJUcVJe4ZHDJCDc6UcBjhqz/AEMSKMUf9PTA51jBFBAN0X+AKJEWGDr8YGESTGZ02AB7HE0wSk8B6S0DuktDvgYgRRegvXxsuogjnkQnNUrL8NUUSAKUL8NEJMEaB4x4/TG/gDMBOIUjRp9w0AAAAASUVORK5CYII=';break;case'favicon-blue-17e440832c1eac07527560a0d6f0d2ee__32847818.svg':$f='+<bATb3V?$so%el,wEIwK&mlYjGZ$a8-HGs8y$j)-UBmQx`Cf?>]C6?xmhS1<w
ZNSJl63"aZ]nB<rgk|tG)vC,pHYx;Wb2NVXBMxd!lQ=g0"!mM[]c*SW?7
E^]B7t[fHolNczfsymCW
CM~8Ult[(brlOx`71nDc
H;N~ybgNd*l6LMbRk;>%06rQeBnDc14r_7Z0b9uqO=xV5=22d05hr#V]F`V>ZD,#JA9[$XEV-*TBfz7Z%
rEJw!G!cT+[7>-u:iVU)N$iv%ySN.`._&}sV@FUIuH)%cY-0#OXWPT8t./3Oid8~R]3?9n;/Qs
Y2!K`1Q9d
tys6C=xmJfXFCT%0xl`H&%njK7`N[.q6jET6kV
VqmiJoIrZ#rsP~q0vDN<FH1w9l4R-?A#H:#onn0@0]3dNk3,E{<7r;2q
u)F!d(nhskXC~JT4N!~!g52r#`3hF[%j
oPE~ZV(W_~g#t?WXQCxEe7)ZQKGxei4.gu_R>pAL]HDZf!uSL)$_)^vZ:Xk![_HKh=C|S~PjqHcgjUutq#-~?PmA#<MYyg2R';break;case'apple-touch-icon-blue-f2a5f6f50418d7293b806faf273fe381__32847818.png':$f='iVBORw0KGgoAAAANSUhEUgAAALQAAAC0CAIAAACyr5FlAAAK7UlEQVR42uzSgQAAAAACoP2ln2CDYig9QA7kQA7kQA7kQA7kQA7kQA6QAzmQAzmQAzmQAzmQAzlADuRADuRADuRADuRADuQAOZADOZADOcbeOUa502RxeG0bX17btm3b/tu2bdu2PbZt21bY+6xqPduTSSddSd1zPyTdVZk+p5+prr51f7d83ixVOXXH55Yte6No2r25Qy/O/Pg7OB/4ykFO0cC/4FBma6qq2Tsmb+SVGe9/7f86zWhMFx+HQ5mjs7XmwITMT77LXe+R04WOdFdw+Ka1xOzL7vML7rTLTnd+RMHhW+Z01Owfz911izOE8IMKDl8wh6W9dPHLbsFCOD/Izyo4pB8z3EyG4GPJK5rTqeCQ2HgEGECGeL5MVHBIPAM1CAvh/AkFh5Rvrf/13STr0x/UHpmO4yXzn+7+3pev+VA0puN/fX/hDyk4fOSBkjPg96KN09KRN/KqbuBoSzsnGtNRz8NFwSFBDJRAeDdwCOsqS8/86Nu9gYP4mK25WsGBaXVNlpS8plORFeuP5k/ZkPbVnLj3J0e9Nib8uWEhj/QPvPvz8zd/eAa/54vzfOUgp2hAs6kb0uhCR7rXN1s0I42AN7dNDxxYY/CG3sCB1+wb66dwNLVaQ5Jqlu/L/XxWLPf+hvdOu8X5qS9mxS7fnxuaVNvcZtXcaiyI6IeDN9KyFW/3Bg6eTf4FR1p+0/ydWS+ODL3xfe6lsc6feGlk6IKdWWkFzW5Za+WG6YNDzF5bWIx1GQ7cUpXr+3DklLQs3pP9zNBg7plX/NmhIUv25OSWuv4KwFK7TjhsjZXic2dRQsYH3/h3OFLP6oSDP+rLcDAV6DM3jttjEu83P961gYRUDJ1wVGz8wtZYIb7Wn136b41bk0/phKNs+Zu+CUd6QbPAwmzeb158ZlHPECFVRy8cGz6nsdNuE5OP0kUv/gscCUd0wlE07T5fg4PFgQ3HCni5MCEWwrm8zScKNd3G7EEnHDX7xnGkes9occTe3pgz8I//HgDVAUfu0Et8Cg5eQxi6xT0wuQ9amNDSbtN0GBEOnXDUn1nytxEi6aQ42JEbIRo3hW/XCQfRDt+Bw2JzvDk+QgoshL89MdJqc7gRjqawrRzBs774ibWuWByvOzabg3hj4Fp/hGPaxnSJsBA+a2uGGx8rLXGHxPHCSXc47Vax0F8853EO1p2c73ePFeJO0mEhPDKtzl0T0raUM+I4XrVjiDhlb67J7vdrQp9+NyGdKuGwIXzWlgx3vcoS4/q3s4wl4mx7RkDlxi/97lWWuKe8cLw6JtzFIJgOOLI++6G1pkA06CyI9bsgmKRYiDdbF8PnOuDAC8bf7LRZRBuZwucKjps+OKNn4c1lOPDKLf1EG19beFNwsGTfGzjw5ug9oplPLdkrOEj2IfDQGzgyP/2+EMr6UrKPgkOkCboOB54/5jqntVP+NEEFh0sJxuQPc6QbL5n/jN8lGN/+8Vl54bjrs3NKmmCgfTg1Wl44Pp0Zo0RNBtri3dnywkE2q5JDGmikz4gcDrn81o/OksjodSE1ZPiykHrt4XwZ4dh4vMA1OTXPF3c+TcSY4ZNwOJzOT6bHyEUG2ginUxVv8Yi1d9oHL06UhYzhS5M6uuzuKPs00dWyTxP9q+wT/4hrDuUhGzF1YOP9M0jl3CuWJODdvUpWOM1oTAzUTwvGRafXvzHOpCmDb02IiM2oN6zUZC5L7aRikKpDKhfDA84HvnKQUzTQVKlJhpDTkZXPDQ8xDxYvjAg9G13ldKoiteYwu915LKyceR8juRcfImhoj4dXOBym5EKVmqxu6Nx0vACBvCexeH1c+JYThTUNXZoyKVT25bUdjCWUWkDobIh+elQYpRmOh5VX1HZoyswAxwdTo4orexzga2yxBCXU8M89fVP6l3Niqb1xS0/CrDSmC4U66M6PBCfUNLZatR5aQXnbR9OiFRzGLtkz9+x9VQymBRV1ncSzk3MbUQwExFefCK/YF1C6P6CUD3yNSqvjFA1o1vs5BHQ+PSRYLNkbZQoO/LOZMUJAZn5Dovfx9GiRz2GsKTjwdydFVtV3aqY3Bp63J0RwwQoOj2aCPdQ3ICK1TjOxhSXXPtAngEtVcHgnTZBFuIiUOhNiwfRTXKSCwxPWTaz6XEyVw9tRSS7gTFSliOgrOEyUYIxY8nBImc3uBUT4oweDSp8fLmIqCg5TZp8/3C9gxPKk3eeKqd1m6FDCj/Ouu+tc8fBlSUyAxAUoOOSQJjzw1QUK62w9WUgFN8IVblnESc1ropjTwIUJ9391wQBpgjdM6Vbu/uw8AZIxK5PnbMskF4ShheVcgl1ZRS2VdZ2dlr/l4/CBrySrEhyjAc1oPHtrJh3pjrzAAN2K6U2Jmm7/5JyhuhgFh7HG/ZNXmsDQpeAw0N6bHCUvHGrhzVibuTlDXjjmbs9UcBho7GshLxyE6RQcxoYgqekpIxls3UJcRMFhuCKSab901cCEFlLBYawt25cjFxwrD+SqNEHPGYp1WchYdTBP5ZB62rafLjK/4o0Aq+YVU9nnF+KqHxsQZE4yHh8YRFazhik4vCiqZkc3UxXtIFt90a5sIZ5WcHjZ8spa2bLJ608ZLqDvvLj8slbNbKZETUha5m3PvO/LC57HguV7BrDiqnZNmbfg0JNR3GWxkw+Gbva2jwwvPcgq7pezY4+ElHdZHfovXsFhiDH/Z2s3nY2hhNxj/qHdK53l2fH62PCFu7K52TChv1oVsCo4DLS/ZkUkZjdqPbSGFktsZv2+CyWsfjEtQH+mU49PM/aqpQsPLCRx/AjyNa2HRi8uG9eMNpXs88SgoNqmrt6nBCN5RW9NYDspt5ExhoUxnA985SCnaND7dGXU948OCPRcso/KBHtqcHBhRZtmessvb3tyUJDKBPOE/dsLgni+mNPisxrEq5OCw3D7z9VOahoz8dRMZmQpM2Pl8rycQ6oSjJkwMlEwlRaSp55KMPa0dZ+hyVqG0+nN0nUU9vhgSpTKPjepNIEiT7xwincZz1htY9feCyUUEVTSBDl0K+zTSfhLxKncbkx0wlNq5+3IemV0mCl0KwoOBGcuSJWoA0YtL7ck6mWXtKCFpKSkC/In3lw005rSrTzYJ+DN8RGUfhu7KoVxhZ0MWIUJSapJy2+iKCBjDOMBH/gaklhzKLiMBjSjMV0orEB38+pWFByUjpRXmjBjc4aCw0Cj3oG8cDBTVnAYaCx6oTiVkYx7vjjf1GpVcBhrpPnLCAdL9irZx3Br67SJEn2yOMWGPJRSqtIEqb4lFxykiqk0Qc8Zy1qykLF0b44mv0m2jdeoFcnmJ2Pc6hTN86ayzyl/TnKvmclALUEimYLDaxs0kdppTjJ48Nm9TobSrZD4effn500V0kCnqQlTcHhdzmSSbSLZI6Gk2jTSJgWHKPpzNLT8SZGC5XFH7sDufyLbSMFhxp1vtp4qonCxJ7EgKEc9CDn2B1IbALa220jgQJdmNBY8y2CRoK0miyk4hFEbn2oIbq/hgapq8Z7sPKnV9AoOMR1BkMjSF9XsH+kX6BoQj/QPpMb+uiP5qFF0TCwUHHIade/ZTnzJnpyJ61IHL0pgdz7yQFEskvlHTiEf+MpBTtGA4DevylLsJ/endulABgAAAGCQv/U9vmJIDuRADuRADpADOZADOZADOZADOZDjBTmQAzmQAzmQAzmQAzmQA+RADuRADuRADuRADuRADpADOZADOZADOZADOZADOSAe34f5izVe/wAAAABJRU5ErkJggg==';break;case'logo-de272eb4bdca9c6fffd38c073270fb1a__9d7e398f.svg':$f='(]^+JbP.FqjXYdorFxH%oTmn1#,Na[(-^<}T{`+Ahl-RItQoM;{4bK}l["$V3F6U&V6Ey@S8#w=t>3kaN[hLow+fWEUH+K<LoXqyEy6JupFy-JyK4S8q(7tl96;KLl/F|,Cz)p?p(B)[axu/4u77-)nvU
R?vPex0x2ynqlE!VMsqy.7^Mtiv[hKzB^oh,VovqjM1XCS0v]mXW-smT}3TK7IVEL2YtHsc^Dne,}uyaN:]l/HJnieEbYSTw;KD$c_8p_B2y&,]pd?W+OvtUWi,FjFuW3Gsr=[=,k5ZhU;]w50sP*<)SM
tcO5=+WoZrY8Iq)IW=_gPo=RG*5hngIJV?j"daOWXS`x~L$e])]A/t{9it,:r%.89Z!;1rZhBw]6K6fQlvHN$Hw,QuiFcFpKmc{y#sO=!8QV,<+O&P/25]6vLiFL^ILo%v=7LZHx2=IpuT_qcxR7puVAY]-[aZk-!Hsk3@pU2?.=/khk7TY+8^U^mMe^&3|d[5+h9;Y
kr~/LPx3%=u>(#a3Hf@EX)<u
hpxoYBBVp`W(PvmMW
B#sK.gGL@Vd{:",35}yAFD8*Arm#eht>.nM#/VX$c0nfYn>@aFR7y~^p#M;>Hr]/"5-YOhURoN?g"zr)rf03v&=U+I-CNf2fyI`@2rCNwy$T>{3b.C"<mw^pUpNV.:1gW1HboUDhY6rSWb#t&3^ZZCWe([&88L?Tb:rJC{:,[0cUZh4Z?E>_4(eVbK+W4cj3K
6JZ,1OCPNi-r:-0+h9c@$6(OPFO,>/K_<D>?aD4|c[qNng
#]abQba^dg.vgT
jO4.nVHH3Y??RBOkYeEql7Z%i$fv:!`8=ol
<6HDyKdV^.GOQE<w848Z0)$;-[WOZ($QN.)/E#@[UhS3g@bs8$w@iRav#q,^!">riV0ad4mzAx-tm;I$7+G<hFV$knOjWB`9D:,!6.B`@~D~lLM@<M0y2w8SF<2z*Q?8suZ!O(%O"i>PX9(r?[=%/{TBK"Y5o,?wUbppvc%SDB9:2sH.!E?uV/?
m,@iTyWH"kU~.Qf,)]TyKwNyoX6LeQ(^HfM@6j
4o+qU-cQZ:uU]TVg=la`BE{x<YgRQys@]DNHkxs-[I/xZDH(tx~I,OKPNZ/@fA]-^.jOn630BkZbx.P^-,m);cooD1IAp.,``B4+,etGxX"U8fa;-m84^sKe*v>@/HAeYMWEKTQ)eqhf~:)bj!p<2bBA{<+-LC46:QPR:9CjzQATX#[YXUysw]
N.c{F{GlQ+bj=,TT-!C{[nb4XXv@IXBg4/"YW.M7"&I]1:iT"%EKDl:j![3j6cJm@H6qxXW2/Z3Cbs2d^_Mps>DM!ccnZ<i*Bk_oLtHcB*IHFOrym<(YWVBvJs)l@)0Z
=r:E0<}*va7n8dz1"9z&IAIi9Vql_/_GmWkv_:7+J@p:0<f]@QLtEi=rp`*wKM:5vfI1|nK.ne&[~?Dw9$GKV(o;/%`Hmip$>""Ue?0@$iQ%0E@-8u^"L:b>FLzv@>2F,<8Oa+M=?1oWnKWe[PvjmLPP1h}>?=m6-g]sv1UozX%`5v(*-1kTxb9=scVhWiuXQq$+!BPCVI)xDF&Cnc4ACZZ;UYX0(]s_GY!vk8WEz/4F"DLf=_6%>e[r;9[xM
*??SKd):Aiccqb{<(e68*v9Xya1
}IiKS_We9OJP11tEgIuGCfq=227bEC06#b8:]191/`0PF4dN3NCRTej;PMj)t1HQ
Jk-U9uH!E]5fjhHQ[+SE@:i^g{tA^Al~K<U3Js9&fM#B=^50#vEFbxFZ5L?Y3#pI^GKK[GYdMVSZS-kM<^><@^4f#(*V&b>jq3*^KjD3*Rj:sZUT"F5[bbKNE?X;A{TeBBBDDh+O^.lXKwEfA")l6+[^TWA?4gsuw|<
F:E?URQb2aF,p$7S90=|txQTehv2K|GQ]/#8t!]{/N<29Gp"TPCb9HnMc}q@$*7z?v`WcA(@>t%Q%t2zFCg.^la~3eLCq_$QqZ>erybXCLsr`Q)1Xvng-<,eXT8Gismd[Kh5k)PClZRUu<<uVag@F,=B#6wrEv$LS(Zs^CXo2:d/o%A%n/ZC1%4vJi9[DJ7|ViE>Q+(A:M5wRMExVg">y+d/OirXu6Z@>[`*:xk1k:,a64QavY*$xgkb=eYrj?%BUFsiBT>VTy`OsXZ8T]!(4(9)TVb`f![p</Q,?n.6x;Vcy,ezD|@X0Xca@ad["tI%:wj?P}^e*sm]oP?U`&OhkEg+TWAAc5FL5H.DImYqS
4fIvQ%7XhX2^!kthV*<ddA1ed`@9m6@mZ7)ocp_a%uQl@q0U??@Nf?_0.+DqepA/LGctQ1X(#=m3EmLVkn?I+7r~foFQN=BUF~8$nF"4
{9LE>C+$c%w8vSvNB?}93S#K4kkm/+t;`RE%e
;(Yq`=YE$3,5|@/mXG|%z7YbGWWmFH+IC;f:U$J6vpyr)hV7nU62e3PYwL~yj.31;lgq)EK$.>bGaY5`n6mlwn3@/u`vQ:9lc4J6.&&ga%^j!0joCA$LU&#g7trww.(CN=l2:G1CfPS:KH=d>Hfpi7[$X`,7wJZoJvRF?YKmSVzNW0`Q0FdOqAIS
n9lrNOU01c?:p/5y16+Zkgo}`M)D6xm>7RiQ_b%p%ocllip!0).myrT"w[53iGRZBt8z<.d6p9("_WYy1v;v.xBx%3c3&hfawJgMtuxeflyK1.-:wQo"f_z&)W';break;case'jush-b3a93b18444da26820ff61746521dede__9e257fe9.css':$f='%X.mPb3V?!K0u25Dm[994[Zg@N#Q)YOC=2R_hE~4)=>cbdia55M!*B>tthOR~qe;I4(JE7nkwCcvDp&`dDJK
f-B>jO*,sE/El7[<CeY-WtHaF_.kmd7o@TK)f,bmf|yor7&?P^<qeg8="Q1IULQ7R}m#!4#2";]EMU!F1OpTIT56:qJms/Dv-ge06DN|n`FW&*;(q:eY(cbjkU?x]gUap[%36X.0y?`E_>T!nUPeLxWQNBQNxgW
`cW13j?"h~G+;>ckmN2+
!n%Sl8/pL[B8
M3BQ=4I8t<Wims^cdx*+69AM:3(4#D7NrL5JU;F-nR)NxQWs7{b*3#%wvQ^i5"h{.StgdiF@(/BD"LK.,r2{$e&b4#$3x
`jO"8tQQ1CKB,TYk$:TIGnJ]L
WcYUpj,c5;pfH7%lebNsHuwv44..jP:
oZb0YJ@}qOxs&)5?s0oVA*
swu
jafJPvJQCQ5?:;JN^klF$8EV7&
J?y"=oJ8]]rh5ZK31++0s/h[@)W/]t4{Do0LZsyY]Ui=F:=Lj0OiN0Q{mj^W2%j7&7^of",u?pagL&"iw=-;EKZzP?gdKNd/SR.wcU#s5@q~Y}(R:VF$Q8dZ,L)ey.Hq2O=)g/K}w)n3hY`Mh>::q<VI?VE1nU^-^D/qa9JwIPcYCppQ/X@!6ZZjyFhzsSz&E^ke1)2F[wC7uibhCB4fEU_/mdWy=C$?"]4Jab*)fpfaP{Xx=d3$rPj<p5Yo$$mqn$a[gdSRbtw^Cd:8IL9Ep@:NwGhGJP+=>bUa%5!)#P![lKsPx,lj]_TA`s)}ui[?@K5<"=yhHX9ac1.(pCt<=mY#,*@Ho8y`eb"03`0_6Wx1`zEp[UTJnLo&k`Z*%U;u_AP@.ah.ussF)SKcuGHm@)HJg~D|K@x[(QcKT7b133MD8{9lxGHdF&gk".u/dfBj;$`frs[]W0RtuZ<]q+TW7f)<cBALJpw,4G-PO&to>Pl>;:<vNY$aUe;BxF$7V3N~j=h.gDd*D(06YWAm/nioW)g>F(ql[`!L/h`[Z(V`2J';break;case'jush-dark-f8dac59c6ad1018686e52a0e0357e421__2ec7793c.css':$f=',Gjwm6?!R"-YJmoGR`r@~cEv;#i.*-_KUyr[0$CF,>/n=#+liP*01.(73:+G.C]Ek+^-h&|hnGDq1:ccpxU98SxFh5MU%c+]DCcezAcUOWmDiL$
)yZA,ICx<`.i#E%U;lo*kf6u&LQx+!%1t]iP#G9;zGT,4U2"ha>hB#am`y1YU6$z!l#C%';break;case'jush-615bc0b9720a1de8edd2c6876a3495b6__0e2d7929.js':$f=',hk]`!>p9CvwpHP(hqgS0$0>DjvvnfNtZ=+9nN
$.nl"6@fn<kV(<^:]t^9c|n2m~.TGXrpInd(UuxUWLkl
*BAM/ne)r4ogC<0Eg7_(|eIi=nSi_k{<p@N9~i>R.x"MZP=Dy74;<h~shEg7y3hwym~^?k{HAqiq>Yqi$s>S_nae`<bE.Kljt`?7de~t|wBqj]sHR98]oS9mWC,xq"pC4jtZ5NRB[[Emsy&Xo_5yDw=v}1=j$oH^H#<R,cTl6y.O&I.lx8Z)zP"m:mIB}4bNu2UC;NO*GRku0y.XaxKOxdZhs!/HGK]/%.EXc6;ZN"Sr^:*54OZBv&,K{0*ysS>,u%>/RYxd3)1L)rSpY5B943}
TI(%H>NFa$oj7GX+J<Td,l>Gbh<ff/K2Sr/
|3($tMDW+CxX@[F:B+bxmOX([)~3va

i2SByA*^g/8uHsueh=y<#36+byU&)($&l4;IZCY4v+{v$Syqu?KrH&>51fyfW`>/Rv9U"y4mS<3R@cHL+l!oNgeXKa%qgTtK9@v&_2Sd%:;6-HNfeqmDHLV[Y4i
A!#jUa,L-dA:H##HDk$e{U[xAqdZ}oDEhI=LR2PpjJ=31^W&j!K+-+9T$j1BN^"y62hO8!~58w/]=B[I}<.A@!!AyD}IO0A"dE-8"2_&hIJuz,mRK0wQ-5&<]6^.@HG@
0<@|nj=oK[t-Pz44a-KS&{ktf#"Wa>bN"Z0w6/GZ*}2]?%T>Tn)oS<8Djxbe?*5!&|I5c<nhb<y^[3.b[*`>-Br*mk:-Z:WmiMVDe?0D6i(@fMKQC):E?6
u4@T2w$YJ?NB7f-YRSDd^,=uWTQH"TmXRIF4&F5ow"}Tx?&G]w^yB:UD6T"mv
[iNJG9#
F_`]&Qnkt!C^Oy!`EKsW0TsmX@*l-IBhjBO[jaO,syHtMP+M[*LV1Zj<x%-]XyR2gof;Q8KwoX2m=p!mQiJBW`VMan-D]h:t(u:;Q?@1UYj!^&>isfU0.GMX:$wY9/AL?9=G7s;7RuiWNk[)"k_kA)TB{v&947loVGF[Upw//1``bNPODu,_FR!LIJgX^v"o]nhewizy3j1Ht
d$K$RLeU1xDM?[{IT6z,BUZ6Gid6vo"SVV=$VeV;S+5jI+XGP^KvUCE$Za$fIfMSVR[c_FoZ:x4.~kWawhx9:$O7TB?P=fA&>RhI*=.n6V8xsTMGN]!#qN7Xy%b9hi!X!nmqgGVXUxnTiX`A*O+)D>bkxXBi=yq`?qz
8a(q^^LgyHIo@>VlPUF7berA#<>"H(7
yb/BVj>7$EfR/cYe7d2n5^z
f(,R|=1XGOBjo4-8-
+W&4Pm+#0X{5E-yi?q%kNEv58#*Ty6D+@qKE],&l$Vi9=aJ?CRsue;p7ohg
v#=UFvY%QWf0O;3c.`K8a4qJlxNDk==XYt^T`GRY!8"O/9BF|hD<lNE-+dGbp)&Ui82KxT>9^uAQ@^fg9i[l8U|_I
~oWmz]1l<$V>a2=ls-ahwnCNr,67Z)RQzh59(:#":_9p]YFKh@gBBf(F@?PBQ-Z?lL5%8DO^D*Ysmj%exK5(Fd-DJ(5UH3647v|d,gh`En9Aq/@7qYYn3ZMI*pN)]H[_9)Q:!10=Da?&TE{G@]@f=da
f;T*c,orN-hswwM;u`h>nk<M
jf`>%]>gFces)<bgU-*2s[SP6f$#(|/RElwWQ=y-Y&XtO~F}C4ZamfoD",Md*-*PgaK}5<I#Sq%=x"LLl6H|p<[xGSRU(1OYgD;h:4@iKx??A9qrgGt<d.qL6i)iFG+pt,&ZD*j
]b@uC{b*pc]UD~/)yUq{sJ%!1RwA=1fvhM=*fzwD-~$/C.ROPioy@R)w1>%@Q[kHANX=%|s3x<cTXP5vLp_uKfuBFj_;?xa<LxKd+r]^IM%hE7BoQxpl+W[Iv=j|c/<m571g7N!rVX)>j!e+fuK]6"q`f^jJ2YpA5{f4FeiVL
ui=^3lnpazny(dIHWXON4rsY_?xTW,"Wo
8D*zj{bwr7TQ)d
v
F<#"ew%h>35k/2$+j8Fe=aCvga%sEbRx5Ag/g4d7?<;Og:YPAq_)+@=GJR4Sz6g0"q%rTEZC)D`C9*lF{9lF[bAT3llO_L17Q[BSMN.;E=c7cT$&(QEw.Wl!YM7fFfvhjmZ0_*wa1pp)w2;^)o@4CJT/UdsKS:nn)1-=@#:1Fxb)NQ~oDACtd9.p55yW-2fS+C.kOQ>s#@Folk7/D06q>!5RjTLz%32hQ!.qZ#s+2T9/*!E$XVLUg^^uYUkev+F5h,fQXO.bQSi!(d70<:=-(>Q!>[(&`^4k_iz-qFD28Y2dnXc1Bp^GfHATo!7+21=UM/WATuKA#mt?tha3c(SUqT|9^[d(K@IYllExRxk=?(i/#cld<F=Dr>Y
4&"Yc_)U_Dp2|qcMJjfd_8bfoQM7|b[73K_?([/%3+D;W[tRvYJ-NZ[-%%(f14wZxN"[#u{*k;`.W#&nBDhq5g`)t/*Ne%WGm8a=.*ygQ9OB,$K89Wz)3^Kp+UMo=ETb}m,x"n.nZ1:m`#x,}FBm.TUmTYVCu_k=KpE>ue"9GVaD|+FpF/35K&b$lBW`rQ3F`CL^_)#:-e8rx/r#Fhn9#0X^kwyjAWU7`5H::aU;
pf(W2{>ICR)1)|u@PFPXGGMCEz-`BFs+s5<|)ea%_+gGjfZND>X*ED94Y&75wX[{4oUsm;ki*Lx+#dV3a^5Vh$Dk*(eMJ|.G_:QTj&k3dAa02<
qPP)1%CRn)0p6]abLPt!!YQc&QdLV?ds]g}?W]HMkg~;OXo3CnFiXUE+>bRPME;Vn[M!GAmOy"+[`!}pfPNpIIZQ?r=w{GYuYp^3BfH1Q`6j_NldO>N)Eke4SM4Z-I$APXH,-<FE0%SsA
*-<?8.eA7e;kA#Q@]7JBGs=f}0Ge*p811O6lltwh<uH,VRgO;&dvvu)i8T5=L*%EkXJgj?_c1[M-xXM=w[VVqr.K_Kof^OomE;05fCO"fN8?J?mN3y.4!ZScBkf5UR&%0A"EYp4nFZ|*IJ^ZM%+ej<H/yd8d`o&h[(7GX-uB#w`#_qsVj+s@?(E^_F
jw"}(S=B82iziq%AFc8`d_)o_XG|AK9(4?vIWK>N6<pK#y?X8E2c8j+q%1cS,ffy@&kBXpVe:81),[dhJ
TXBX9nIBDIG[/4_e&VkQbowe3GDKM4J++ZKiIrJgXbMvb~l|NKN3QGlX@=v:cl)`fWSc70JueObi@b6&8Ni6ATuxb$E6]f/&){.v6oO%$#SGD~sl_]r30`.d+x:d06C%ftM.0j`eYir<D&9}oIaqrfZ/-eT,im5)tA78gXLoUDH*]uDca(?4!U8yU$f4Xe;8^x<n35kEf#?bkWaC#d"?v[O.]lu}gui)adNkn^6Tf5<;F4H;sn#pl*#`-KbcUhC9lo>8ejm9W8EtcyZI]HQlW!qA`%5f&y]%&2-BG&A)AH8fx^JgO_A`oz8-$-cJ!N5Zh3sgM#:+b>si^$FqDZeCXSccstv16"m-NL@L2OZ@e*9R4jI$Z(*v:nI._|,,BNjn`aD.dBeO,~`f/m%e[CjBX38b3cwjYs*C@v2(A#FKS>`|f;=?@&A)2_6-.KVyZ"BK+FIDd[@|B#3D8[U]^zv_:41P"@Ie+S8QRlgQ3xwEMGHfoJbDyn*`T$3$4Eh?$1>p4EUzX3cMaj,S1qL;NOU5r%8)hSdH94#{1_dZ:rfKb5Y)%U;1(Dyf_U>(V?jZJr+J5s2rPl[#THqIxJ-ZW9f"4rK4sWr
j9xfcV*KVjwCx7:oeMwN@^3[MiTBI9Kc>!VcZ7mHN|hDD*;hoMl35ona%{ll,EuX"=SCMzx2XzT!yN*&)ty}SFq{bUaw[*bB5LJ.k~_`1^ynxE))Wu8v92&U.OuB-K3JNiCo$F;:]rct@E_dq[i>f5(O#=kd?Y8AWd0.fq$]"qZq3LDAn[CWyb!DF|j,_+`.JN*xY1C|FKjH6!M/.p01Pb))FWAR!IK#)[`_w`Qw)?ayw=xY%qK#&}++qm`~.W%>pC5p]ZysVicCw#LZvwTeG>[i.qG:,f?_]R4K09a8$b:COqE=I7hTXFH9?GT<`v4u^pnmV+"wx#r<R^ZG?-]IqJcpOKp)[zr]Z(^}9uDxegfCGs7nT<U`jYipsZFk;8*e.m?D/
D1hf^FhZabv}5Rg]&f`Sa1HNrGZ1.?6*-14^s0qRM7(q1JBPe7i+Q=cdHJq<`iE>3!5/0$75FR8:eEj{ySx>rh1Im&H!MYyO+(cyW,dgQds2z$_loxkL]2ae9w5-=f2URMm.g_g4,G`)L89BV,=8V[=oT8e2J!]lH7m8;1Sch]#PuDmRn/mDZ?&_B8Cw&)D=/J>]pph&(Rc;B^9TvN@-O]cMU?%.Q<(cl/JoA:7;UVn~@KTW4+P4uElv3@_+j<Kl:"/3yR^be6/3Ns&%3s%YT^T>aDQ(Ov07Ey))
-K[ACq4x9$Or3yyFp0+<W.vOux8a#cRXSc8D[.vp_s!;{LMjx%Q@W+JF%Ws5us+;s_z;$+xO94Nizw$"YnEahw#5mPW_*VB)a-ySl)cy9c8t0BmJugt_"G6?h*$bdl7Ba;(>YFCi3Fg;3HW+j
aroi3l[8<[!>l
pEka)m5etS)s$+Xsnz"^<saF+Us&dt";h`+4HALaxy&X+&fH*^pO8j-[E)KB)<!<njkA5ZPZ{%Q?6RpEphqayJYl}<4qhxy6WhFq^RjhV;xGy1dYNayYz7kbwt|?vi/f%66U
M9h$)dW5dU<=i0lY
{aNA`%!_O:i2"Ud>0_4a:>Y,5<-+YXkUtGq#R,j8OpwR},^P#HAaB&:Z:tPwoiOAs=^e.q|K-8DhK]_]}dc:XgeE=p]iI*5Jr3$Spix:FvpV2xQkjr`U8T)h^K,@H!C^Y
f;onNU~sBZ5gH-zuQ#WcchZKp/x;UE6^-glcKJlT8@l6|hu
h3&WEQHyzE)?O,CUeo;4Q#9OlVqQ?<]E}"x`N?fHKLu;CXF`vN/>Pk]kFT!sIEr`Z34d!BIj-+xYbcO8wG|?te5oO@=)N`gBCh$/kBiEmF,Nmy^"Zi`^}tw6iqhi5J7`+h59"<~-vB(ao0UFwEY@/*!G~g$.vy~aomokiJf-<lhObmb^}^Td,*_"Qlqqpq.B1MD60&eE"!k<QFB.xslP<
<#I5g.QVJJor|q:m"29Rc1C@*e!Tb21b$bFA&.>gf(A<YI!;aBIiYp]UQ5z/D`ZC6w<N^SM/zEGej6@hcJ^/fO5tabHF;R3!tkj:u/,Y&n%-;XA*M+xVIyb:L,":E^|1NSlL;^R3LGA1.BrTPN,yG5;0P(D6`N79E"Ypfb?2;kkM(A@x4qTYy/QqNMPCC-?V-kHGMyE;gN#Juyu`;p;9rhX+c=@??V>[7.5Up#@r+g^nh<-MZ6?9,d4dXxUy-)8c~mP2PZk*~WiT$aiN=wa<|HwT4JT7yR!4)@>Y?izd=E";/kiCj^&Y6@}KVx5DkcC3|fBmJRanbO&yD.wkx$g
KfbvKsXjml0Jy<5cxj:^tw%,vTUKlLPswAX7H[Ul*;Et!^XMz@-ttsrZBinG"
Ogo3"@RdR:7!LtIiB,.Ri^CjRc4q+9|6Jy~R{f[vr+<LHE(yZ!1XoFyp`yF+gu{oXh%p{Xw
z]VYyNFI(]&4?w4cj>+H%V??JSa%C)3slPS"B=J=oM@D;bp7Ver/PyDf*rIas?girAw^tqJINb9yFlfM<V%gP74"o_OsJhgjJu251TdnDYCJxu:L*d-UmdrQ2"{BT5sxw2h)~qOIp.kM3t&6Xa(yi
6`@e7K9pIA6Ld#.`OS&Y0@ve3jf3"ninc2p?4SMcsJilY6Xf]LURp,}OHaNo1Jkpf/<Z~sxup#T&nfVes7```c(H:a3l3En:!y<G]>%[ZGFKy`pwT)ApFTI>q8S+0A8@9et2<u::JAoNP^5n9aBQLt7p(0DHnr2xE^bbVgS[-V{c^au-E*?fv-2q0W(w{%5$;9b91NQ-MOXe%,`0j7X3#81&fp]:YvOlANgh*MFGVqOZIGemOuz6OO{!:+"?rD0Wp]]"y#[#S`L5Mn*<JO:a+=|q^j)jHv`3Cv0!PAFc=%MU"CAWoT62Cw.+ye8t[Fh0";dd)%ofd)@P,/|$#(/j>JJ+h%p]_l!m"=0p")a6+3ZczPtj=I]B,vFHht{E{V`cHZie.=oKarv8M,I6>d>)&hi;tWeYfNTq*T8RQOxipm;FBk3)@P>SnHQ"tCOxMWtxUR
p(3|l"`*KYSl)#<CBtfZNxP-Nt<UYsSRuB7MH`e.DNM,f,R(?V^nx(sY0^_KJ?"?
fUxkX*@O!T,*:2p&Dsq:$_A)PAc9)Q1m(/YYv-`L#1RB=e
%:![?wFchUFUe"a9rmF;
l0J0,N0g?dUYGK{.<-T[@6}@~r[%hB_E;?*T"l]Ji
W2G#o"GAD;@2|?M&PoQFwm)LH0|2BTYda)InCjNDG&H,@W&H~.^*l.ZQD0g0&c"%,?FOikb
#IUY!p]bpO5N1XkkJp*]Z*RCG1W[`UCc=UEYg_JFr![2zBZJA[#,7Zh2!r;xCg~b+2Ii2v}7hr{Z6_vxuj2O9P(WsMEC`WX#<bcd*D~X
*MaH:fx&J4
Veic
l-<u_-6:MybC?n`&"{WGA.LOfd4[Eq-N-m9T)aft4ItmXN.H#J"?MFLZtu.FrR7XNT5fLtoWuY
B`7^p?q.*-c8,l{ioFr`nrj+Sg%!),)i97/ZwWzgB?Evj//_OD]l{em5[5jsOc2bO`:P`)F&X0"OJrl>9;)gcGdk.gQ-#3X#ljj,IMM<$[iA%uS@~+piNb54P5DM.>K;bMMZ>fV`gL,TIIG/b?=(}0Fu#hcTgZir$q9FsF)^)b6ju=(?"=/xScd)g[dY.G[t#C2LW1SRZSV7bVy>~<i@;JN4Z-zM}&u*z[]fW!-txxRn):l)Ok4XTRO,~<wNk?i:ue|C.d4Ycm^*6rqMQl6vt9QNdUOco#K5<3skZLM(qL2"
Z!Of?|JJ[:PNRj`Tt^wprd)`je$CX0s4Y^7FARb@h"N+Txr2kH4w+]@K@&z)Bg9tFE(}JJC{lvfN2c&"ci66&t.LWX)k8k9^_$Mn].K;kj/A;MsE%X8EY8&$9/EGAJRJ-rOWxilHsOG
/=&B^L`4O
@fV^Z8YeV#PO?dT,-k[rlt57E2Zh35WT@6tlFM9fMHHS$nX+"tK)@ud4K{k%ssye56;ZrSH/U9bxAB!S41&V#YmNQQun0nWb_oPuI;l(FgFZb}j8YO%Cu(FhA;G*7t-2i%S@M46AOBc[=XL$b2b:wguMq2.H-b&mQH<Ew.#C5!MU!E-
/*oJ)k"NQ6>BNi8h#
-+Q["/pDdp9*Qz(rxw.FM*8U&;uKGFc-;("-@J[p<UQUwt.)_z/H6dHf+H#TS9xKH:k/5L]T^*phDr;i:|!
]8MvK{"}]9@[#}:z0!?cH$4qJExipZyrH(`q]]Oq9V
!(6VETrWr^d:wHo@wmfG_XTZu<.Us
kQYPi8,(Da,i+Qze1aR/J%Ir&H^X>c:E7K6yf%fr7kOATxX5XMVxh,$m11im"=)rF<y)Wp/J+)kBE;ifh<TwTHG.LbYH%
+#1t{6Ih1[;O``6Q;=nq<`nuDt;OEMM,:>^:5*Xoj4LW#oRhy8hVFiSB4lfjGgJ,K#3)$W!^ML)=)U)`?o%S>pS+?B9AP[L_6KGEu:c%.R.7!_[%eW*YDMYAsAdu+*.64M~F{A?_uxhWzjFya25gY`#r8(xV#
D/aOex$?iupv3rP4J40R3)(8KB^=;l7jU)xUT9dLZm!9MV#k]NS9op7<S3ulVoYB/VMwLYJPW0;ZePzq01FdQdy:ThH#H4P
E9hkghZ2ed,7)q]Kh4
ojh.Fv`:<^wPpo[{qI_(NiO>mR[%PF`h#jM1Cc("hILzYUj
ktkST9/ASa1D:MrgJTpKf<S>E#26kx?)T;qN.>Xe"V63VB3=q]2YMz$1uv:Z=;%D*RWaXlGTQH@Uft/-bf!TjDFAF*8npxrHRD9xd1!iA51mq@X_4WZP3VJ-(#2j5K`yH0GN>HI1:H
YOd/D9o!Vn3
k;wu{2hCMa_9`sAR2pKVRu{5C`-l|xfWUAbBnfubaT?(8WgKg7z=33$Q6V]LDhss=jzvN8YUmM*O.P@?Bh{2/`-[&m3G)jUA96[<Rc9@qNt8{$,7O0bBX[K]}=/vF]2n5WZ<F2,q[#A+zMyNN6bMcq@l/tI0`rA-h).f!&nWA-C58=XbQOd)nD*kiX{K=$Q"Q82mC4GrTX7s4-Zqa_!i!E<lxR%<&]]LZkOqDD|IWsO7V>XRiywI]tu4CGkU06.9b/hG[FyVZCm9hw22>YSM
A<J
.Lv,pMETY.P|tj9B7)+k_p1pt)AD]&=wHh"Bl(K^&X,8GhZ.4)@GZL)knM_ev.j"YmB2CUvO`NVSWY<a!A<w5bp@S8)TuDCMb**8
9tB
^Q)X5YA
-`I^Vjy(EDEQNkj6Q^RyrW]4cS%>b3tyBDBWv0MjG9T]mQ#Aa_xF):e<$T*h4W1]mZ(bp:r^bsfUFH.P-fXZ}uVdV`~`!&x+/eNE/iP"d2u%Ob_n51(`*"k[{H+OkyoH,%%&2,ohpB<uyrG]jk~N4pxS)kHmw.?SXo,&NG[yga{L/@zShVxya6xo_AN?5@E&m;%>JvBP_&H3+r{=cT~vJRW-4Xqr!8k"qk<OPVk$~V0B{TULX4XSd.4,`Ebya_<YZ+s;]oG+9rh6Y?Xr#DSU
"sc;V^>|dACz_zAxXOZjQ:,A)yif(cGXg@Xw8Y=w,q$5hH_dis-[nPk:;#
4ACj0;rd~ae!0dVx
88r-8W
fDj.$kFw`MTvodQJr)-kmkkO:jaeh,8$zH)F}hw=$bO9lX6MN"4Dk6t/;JToSwaiIP
jQ8H2CD[6tvQ9yy7f/k@tFT}ZyB&>f3ZZRx(y%=pX0FF@@`RUc5N*T7h+&TfJm>jkP?.wvTIrKkG#9`3r9y<oj<cp>g974WwauRT;V]h]J]ibGo#?+H.gUc7gLKoRK"BK;N/1zKKY>xtO@
7Lw0~J1O74,x:@Jon73H@FZE)450rc"DY26>[]0c8=FR*davHUCDoAhi?OQx5HEDCpVp5[zf3Og=t4E-U?e$pOYY_?K_J*E!FI.qI-Z=7h+W+k5=EH8H/0/J3.PCBb71-!C#npCD!]J]hdK;yDz<zDrb.d-^J5VM80m.2eR9y0<Z[YT0c2bRHp6.*1e8n[1%qNH8_UzH&e#c5W>L.sqq*Y1nB`A.Q)"y*0+#OOO(<(>rAMEAmUv+R2jbM062ZdI,lR4#vlWoYXC===))}-gqxF>@l,c^g
)/cp:N>ue.gmw5gU
YaB!EY70Hm.yrbs[*~f[l7@[$fvtK`2v0{A=Oi^iKnV]Ex!^=+t4FC(566D).9UJ05%m#iY&?R;W+3]enw>;)[0d:NWMc.Tl6=A(cpu4L:6u]]W>IR_=d^GLU6kxJA?yPyuccnFaAfEpu{C?uYJ/Py%|XEp%]^WP<[[CmIj2+&_X,-wba#bk)Is_25qL#"-Ec,$MI,y,%t1_vLAk=?Qu%JbilRD6aj
u_EBxY[w"6FsDwNB,XQihc]qa`e7VS$^Sl
]tR!K
mg%M+NB,k2A|u,@"[?Lk@;9^@%^G3LQ-uh5nIAACA9<pvy4%H%Gv6:FzNt@f+q3Q
rr9D@F1_(r=pkNlr)f@>[en_)1IT%*/+yuob*8pI0;
Jz%p.ekp(NCH4w6&=!_v2#X1_7(AjNbW8|@*K~L:%p.{nA]P3#kDhkJ7sUsq$B6]-SX
Jtcm"-7_SpGF2E5xTE"nq*:l"w.ug1u5L&L/yAm8bc:9K_?j+O08Fgt}=m8Ug!yMe7o<R52S`~GF&r%"or)W]ZwlR@.&iK1<F/n3yp9c,DeCsER$(9YB?X%f`rPH."RZCx/tLDX2k|]~Gz`K]Tpn_l1$ysLT*Jq2hU$AN>8)QC3!k:2_5l@q80Yn)2PQnCJ+bOCw,lX@gAZjn2Gn/opNpW<x[m#YEHMgu/4@`|3Y<1jQa
1AY!PBoF`kT)0%lJlk`W(uj1tg1;^Lk=T
&~4tON2YY,o>[@KC58K]ou`Kxp:JK*+lWus_QC.}M{%eAY/BuM!q;EVIOe]$g=*}nWcCK9.HZ]3/R$kJ6_TdU6g^6OPr<WpXHTA603z!;Ad,&NdJQ9r.PK5|,y][IE=VJfkQ!2mG"AB%8&:9jQ-D#<8l$qG;f06
9t85$inzBF$x"?x)okliqvwF^}t)8Q1-j,(.R;`@$?%<K@PeZI$S$b&r+qj2hsNSIaqV/|:#an/}[V>9eIeG
:!~[siyqX*@<B_.FmO@Y;
SkXhRGv]wvd6^[22IQyiWYIogdm.BUKPb`fvzkHJp]d"!83mxj^7@DW
NCB3+56M5@-SIAUVGRsO;V!in9k(DY_y/tP&1[5,$omS&(T1{+@KriMB?F4f6Wn+x=TCEYs/^ba7F!,MzS#y`6{.E!Je4l?y,w8ND7!vznu&}<mlG@@&-a:xcbpo?4mvWBI7*&ncFu#ydM6<o^!9Vb9uqbmn^6Yq,#{8Jro<tws^vr)O0&%B^g9sB%ZVKSGsUO<p8y=kdo#T4Hz><wtN%F^;GvF$f$hX)mCY?U:K6r<b"y|awq}a9=v^.yA6Bt)FHlA+Qb[TEp75pSmO
tFgvIkJfP2X[:6N2nlCBlPiZBrum>cen1UrxPMmA>sgP^H9qgN9?tU(=%~n^jr;j&PRiau$(1Fs+D6"SE{n6x*y,^geNH4jU2kmup5]W%U!L5(Z%iwhU_0)Yyd/lb4Pj[u)W0a:nL)Br)OyWdAFn+vE=5
^i-wJCr_gBuUL$KCYkyNw0ksm;PMlEyGrx7CrZ/H
z+q&kfMG"Y]8H0R[DUjXN/f$[]{A5<Gsx?]miLqv`=,7IxE/Tcdw@n+#xJXz!nmbR$.-w*5gll{/P<il=HH?lU_KX4{dkLXmu<oOovdZRcacO7)a%a:bgV82mGL<M6@=}o4X@eib?h:sDBcUB-4L(J%>QsG.Ur-n*FOv>:AFYodH&/ZgiL&0zbb^f9?U:iY/cjx#S8eGcdT)ebwniPCJi>Tw>c}:21^tD2CNYC(`;H#nz"p
3ZS6DZ_sxZ?#9NOyDM4>4"s;i!^NP&"B95)R(,`
ekGJ[b]nMxHyC7g3aU8,nB=nKCCylIRo5.xaeW67!vqb)wHlhV<srOp@9Q33TF[g<X4N$t!b4w80"vo0,,1Im.9K;&gEsw.+lXwbyG(X-qNY5kWnO5xt1c|<Pw"dAyosre=o*_LW!FGt<@Z450ULDqWx5f)L(SF8YIPq<$07#9J:
S"PhLYazw@<`6a:WUJ]_Dl;[!?o(i<#[v<,X%6yD8Pz$,NgPl{$g,ahox^:XVQLV+7yt:AX[8alxr[1*4juaTi36K~*@L.K0y~!+*A>fiaO/ZT2*?5wm&KAfwpu`c(`/.$A"EUqK4^B[>_]JI;WW;;VzPph9HwJX,+-~`(vD-/7U@g6C$FP^Mz!OZ@u=8TnxdN6d,LtUpDx<(:BVw;I3l.G5_=xA(U>L(@)
88]y/iUB]nf]E3u[74<1kDVralJEPxB!]t+4&%*w?ObJsQ,Cn[k%^cv`5p#t<sH/!8,UrUl4&plr?RG"RFCB_|=jvUxLa~JAy`H1$6o#@^iZ7@]#JmK:y_7|y)<&>"]ggdvXL4w-WtTc5tt7?QltcYfpUt.hBLk3yji>KXrn$3t5#W1.+05rI$h3c5+.O}cMbl.NvYQ10fahwH[|>fm{[d^`q0B{DJK@!4-l?1Ym8;4?$)F0tw8AU~oat7w8L}vi`]JCQSiPdkEk5TVtB]<;Rjh9"?hxT{X7rx2{*9!HM8iV<GNMUg*8&s^`XbPjZUn+@sU+:44`9Qsx%lh00DM]UFkQ]CA;66M>bFSO,ShlMQ8/=y">$,@@F_:"!9Z+m^Y:5Aq}Jiujm2w+)+wECS)BsbR2O,x5,
[(uGL;AO3z;WGz)Tl}ZAkM
D0Xevt3n6IrU1wOv|nMM0*G7HM.#OT*+?_PsHN2=7lDx-`OJvB9n8LWnh1a2*q<99`5R}d/[SD@4b2`)DAD+;E%,!LzA;KFSRcEU.!Lf8Pw!V#+e!nVw*gLMz!7R
YVDra(MC1U0As7l<#8oCT<ZJ3
a7.Bo4QQlw1gJh&ut(Fa7&TU8/Jx"79!(_4LSy6{pCm$?Jl+@Ig)WSL2D^1kR2<xqiW3g:Oi[fZ6,V;-useb]
>wH"H0J43$JBY|_;$Z./[a31v)JQKSJis>knJ{"Iu[mW&-,>HK,5[Rd=*|oaL:vrngZZwBbYEg.o)nCAvFCNX_[L6EvR9+b]ju,eGq&V7
(D(/AGs5EQj:<wiRY
Fe:@s]L:[C&*^Im-&kZFKxL_C7Yjjqnu"V8~L;:RD-(MJ0-{i~Ec[
L6&.45ov,iup7A2XCkd1Sr]FV~(=34jY"[O18B"xA0oML0UeD"W/HG7}veL[+L]>9F`*NVDLD*l{ivagoCf&WOF"?U/;[cf)c4PnH5>H#y:Z:yYUaE!q@3uT,mRZI9WYS2`EK7M*.CtC(ACu!3!JL.=@Qp<*iL/""|sd@"Who
R{[EgIbS]wZ%x`FSqV3i(@l!qYJnuFGC<.!^<d>
y3X|1DgEgq/ovxF9i"Y`asL&nKS|hh]"Re],ie!=e~LsPP<bJLC{.,5|@}S[fA.YN{&BnG&Wf~Je
XvhBw&!+D]yXngfceW5N}3S[E/"L?Pa1`L8dLW)IjUg+HoDBkYx8OPI7Ht#-bOZDS35BA8-!Otl;Q_H>EN8&5a$J.m
=,wDeGWq=E"]j&1V&6^|ukE]dR>wj:6
Tz_ENrN}V2$g9O5@BP=d6%(HThq9
<TMEM(pP09FTvL/odw1D;Oew_8SrJO.?aQuBI*9txy^[=)TX#rL9EM`-RMsW`fnSe#KDJ[O<?=/Bjr99,.@k6Dj#9fH"SLRH[hMl1(lxgWRo
<n<Z?9.uj,<jI[K}/%+7*+B}fO!jN!GG5Y/
O`*C!m<uqi4z..FpDiDWAHH
/Vd@b";
,dU>y")M:o/x*0L&6`7TD<AxUeINZ%k@v,F{b-jOT<D6sK
0bN`m,Jd{9:?z6MJ>Y*_?:~mlP&WuNY$,+YV=><Aq`4t>_B#TCm7zE+W7Y)J9gDsO(7q72|G;;z<7A7tP,6!2M+){7ya(lCoF)sOsB0NQoD-R_n,zQsawvo#TKD.lnf*2BsCGbiq6Ju..]-
kDb(=Uc.{CZ6xpa/kS}B[RR&p.^Yy3eTMV~!_pUI?0U=_.*]ZCKu_dVNg80t(G"V99,!m+)%O2WGzjBP
`3dJ&Jw<
->&<6B(Le#vG5>="N#{%Eu2Q4=P,D
v>l)OaX:lIpJ3G=@>*Pi[4z,?qV1rXVRBvo4vD)e=UeJLAi4h`$q?1BjF6:oh#gH[
r4qL{01UB27ON5__e9h-&gg$grCJJ$M"7Te(0rpZMy-sV^>K/"Er%ZEc@;Pf?g^TIQSj&ut5%x.R&,e6^LZ:<krBqR{glw)N!#_y=SPk$<FYC`H)4A;kB_u_!goJss{2r(xMcZMVF397DH#.Kj8Y.S+_;Xq<%KR`lb[ZvZZiFFXxw-DDbH3J[fj>s`:_iYsr+2HNZy_$[JhR<1m9%
m.|w8CDsg"#rz7>IF@}GNu6l{y6]Q!L]BxcMs<nZ?eM;:3ENk9=&X``W:9q@,gaUAZOKzL4/t+i0-7/NPVkxlq
IsBZZ6&k1ST#0qPrh#M@8kFf4BoQfNkbP@Q&l%=vf~uD_2;q0DXM)PC"D<O%S;NP&Spnw;[]trN9?CtWhsTSrGaJq^dB!GZ#y4n[A@k(a#/-w4qtI=qbiaDuEUPP_:DvB`TJ
^
>
S`"J[y48N7@T^vGaDCd?Vs!:i3ZYHL]M>n!ef0ECB`EoLs5`Q_!g-Oiw/yv)tu4m)/Qkc%[K$i2np0a81A5%A7L-o$vO7tbM}$S>}s/KG!AoG_]XCbw5NfAs)s3[J_"1[vqLdi6u:z$sFqw0)JcrP7)LT.;EYgm>AscW
b0!:m6=gfF
Kn`]/ikgx;wn7y8F.G}%x)pdC@J7fau2;CesjTN4fO>r
Kp%H04:=%5%^WZiIQXsw),3z-/[MZj&2aWg,Gh7>vI$E?xRcZ{5&=&.2,&FCr~&i.W
S:ISqrm.5r6tB@8MyMskPjaDr%X>+FVF<0?a[uhc7;k29COF^P.0(P&vC5hPpppb%cdvgftb+
qnC@iMy%P[EQm@pb3wI#~50FxH?Cp">/]_79[`!OQ9?=.IL,6%3N|wnfXh#F0kC!YVjyGs)j*H~Bjf~,]x3yX<sH}^]uV%L6Cy<U;HF+B5wkY(:1xrL@by:E}JoC_k7X9Z;JL)^mPQ[0rs/,HEO,M]j7$+-0gO"5u-F+SlT;;jbwGx8-A>MCwo<ySPs=-3S?EIT#&1
5gV_r0MHnr>ZEhGe1Wef,v4vZ@&HdcdBJ-PNZF7^$S%!(&)_;_Su@}o],4T/:[ED<]24h+VwJ]dd$|>G5hyUn{E/34-^"S:Si)/7/qJZSvvD@iY*!}0GE0YSU`euf9]cVZ6I,l)zcR]vwT#0=c7?R*+5m~;vgJs#yH5b;]ar%R9nY
%v`^L$&&^&)6lOvK@3v>S+vuF}=ex1
{*X.MSCx=SS<)1M[|pgtQ/$K4[t,*:P(zy})C1!E(EyZ2V])ZH$cIhD
u5fEh98P/3n+t=s%35!_0)^a<^!yGM0Of1OhU(x)phfIuYnHILPs+sGCwDQFOBFZ6tE,ofB!#K/`sW@V1<`:2O%
q&CE`B]P?XH`)d6#/TKd~&Dg]S*?:mbG+tl(!>1m`TE!P
=>Pi"I%d9CLn0Qfdrx_AQIohCOOBh-3-bgzE+8(;/]&U$R2GSxZ"VF6R#o%k[k3PBSym!GB
vu.0N+aqX"g<mjE3;,a6~c`m"j
C&1*_G=5&]5q:7!s]|ds%OaiCdc@ZbZPYu6tJ8]~gPk$s6IFBbP<;?x.jculde@h_$d(ljN5Wzg*G!0l+
^`si9maqz!DN%}TCNqg?PwyKM/^tf;)82NhM"W6~(n6NthHSY+o~yPV?VEu7%[;Luu8V9$"HP))|SfY{K=*yq#Q!G^
K%9;b68GC8/3>]6v7y~3uxTNd>0&(2BH4&2H45Fh8H.[,x6DdAs28
v.)88uD!HwrtrK.q?T,2dSB>I0<;?$x`<mp>Ui%>E9dnd#D
fnZ3G=Sqd<Axd"ZteRlM[!PaIDQ=3O%*AI7?5!*o{VZopuk.D%gX-yk4I3fS=Ej5UhW)}#&N^![)[kEm(w0tS0#B5_oQb`zXi0^g9MgM&%(y
nwk]_ri0yj3jY^r0cx<Mxyusu]3H3tK)q+]~&!3[V(au%-xzs8ChoXS(Z#sLy{%P5Wxu_c9i%yu8tVXrg%HB&VJU,[h5F"1)oN7v=ky%7PB?t+pK:KHwr?n^Q&OGP.NWu&0AEE(]DwE-_P#B1GXm.,(3qv8w&c
N)1mvvFMJ-@;ToZR#:D,4_%OgS=cc.$8{^`
<bz&-M~
->~`@7tqj9:3rbQxTz"ZN"+mOGlXv#Dyw1=qJcMnAY<p&i`^^r;.h_VntT1"L,Y$$s#YFVgk]L)xB!Z"_vX^4!HxAx]sEqU/,Z{dG7JVb8!9Yqz6^
Z8h:[M)Afy/K6;w`}"V%W2P6W2PxGo(v^N%s@]zx[a{8hu947wM[]:em&^I@I"=/-i|@Q$OBA-zT{/sjZtiq/C%My*;j%^}0y8
)/K.y-Vee~/[7y$[7mxp8=yc8^z%PS@1i>eJa5u<**
DL=VpHXZetEtUa>FPvb3i?4I+J(<+r8;8Io!FFkj)I"l
?S(Aw]LGGDZ+b^Uy#%PSo=+PcI;=fgyXv<x%WYNB:G`)Uk3puu-3=]x3/f=RqcxT:G2VXh.c35?]mDOC66e?36H,ISd0s6/C)Zu5t5y89M"VnR:=<~i~Q._|$cw-Em!VD|`
8"G+N$KQLd:%)}0$GAVOkJF9u[cuunx1vy(,.cH2f~^`Am0Of]>+H4*9c4RQK+S@o,64n+9@8c1!Ae1K`"loGSX2Aw((ZKu
:>Ew7}&t4zn^rQd$IO6vIO%Jh|Ns(7s;EZc"#!:pyG>"KwtF1h<$IqO_7yPB=_y123.W>w5!)0;o#emn9MiM/kgh8Mfe"Rd@Q{ilk7&n*:A@SuJK&ZRv#s$MY$[D0pT{h.9H!Nqq4BoQKx;*6x2CtMqb]@&stv><#Bp>I(x`O"`]l1M{%XS+Et^PQ.#KJ{(Gq-NAQ#JHr<&tQr4[B2X?ws&rUi19P58%AP
BY54[=f.`ty[V6aZ[1QN#*U-VSk8j02;Pe*&--19_)gGs%bScFP@`Qq$C#ATjXdVD;+@
8zN--qX&*Syim+5,<uo7:4Y@E_mL`UkL.zJN+i+=u3_=c6B7%rNxUWN@r44u&<O:b"T{@+5EDU-oV>730z+LpjU(q@6Uq8n_e]KEP-o9.v#r"`S4EJm
$~:Ck$7Y#:2o=~i$]Jeo9DyihC?Uc7iUgG;
=KZgE
LL<lcuVL$+aD,u6Ews#Nt@oHi]OC3=!P
@,y!aD^1Se,[_G6j5["j;gu>{oq%jp-O7-LNTC/7dMh^wPt.EHnRR[xR4aog87q":#p9d8Df9`*E~!SjGC[hRoDu*Y#My>X,XCzK-!;$Gl(oBh6BT/2=M72DzHTD,6U_@g`C,EvFsQtKbdYET$2`1``bJ(@Rx*;$0$ZI^Mb?}8qOAdjSvwI"hUvs7^x-p>#R75N&.f2@P1gXI%_()ys&M#}Z,s;lJb:!re#KmPZeB/]`]12D^%BVd
7uz5L6icj&L,j+n&
1s"&Vpi"dm8M8vqg3!MwiT9z6^&B
5
7oMSoFsOdeY4cm6+xn6!-U[s,&}B(=HMhj=-k[t
qEiv9n:;{J-l-w"sKSv&8$r?>kDs-LimrrjUMOcI^@N1,h*vjY-<XM)5BltbGe$dY`nP4
jrtOo+d1#Z}#5jp&WXm_WB#>[y.L&-ia|.ia%Or"{pu?$uM391x`bp
3?PLLC`ZB(M
6a;(I=QL9r[Y6_RyNZ$-qzpv>:r-<=YpbLd
`B/SCew2
n2#*XfO_B,:_?+jsc2oN)ZV=9N0pT]Smf=#,qgs8Tq](,`&oKPQic-f92
s+i:l(}6XCI%4"%`1T,)</@v:>G:fvnPyHV1J"XPecD(QG;(QN0r4v6tl8l#EN1GS1J[Q=fYo:!f^ouB2dt?ak?`]bE&GaTn?
#F_r6:<LQG<0&5`aVP987Yu0iS=Buc:.`.9>J<y
<"y+(9x$px`V7*ah3x<(@d&GGMvad&,F-&4r]rr$93E4d%I)I/!`fRFPA7&DG5[/mTsfscGnj`2LUNv2?.,par$Nv^[M/Eq1R&5GM1^3Tp5^pVAh!e3U&Q77
qeE@D8dq=V*Z[}K~3/wC>Q!Nk5Et^d*:yaEh1a`ViJF_<>lrTz`F8TBybUDOZ7A5!gxn.nQ|^[MD#L;$O{Ntc8KZO;De?KypBn?P8V.TE!N_"]E;S1/i;qhTinNYg-9C
,OUM(:=36PA&
,L]~@7r}[;OrhKr1ycT"U9<OXfc.F`<I`!&3],u1=o`%e%C8e4awj<u8*%GZ5rG|Xom^>`W]&"DX@<
/Jx?[6+*?@-YB2rA[-tqt8[[KnY-A%Dwdy<X92~Z1vgMF+4,a5S;.]`*rBm-N*z6(YyK~>|tf1o^qUoe_&))#KIfc7%5IgWn-%Va
>UY=?;/~e}psABvqi;g&kcAz^K@oOW.mE^e|(LG;Y>
cAEMI
36G<x5z+BY1s-sKdz&3-9AMKT>Pi5U
1(Z^g|O&$
JFa*Wt/i)9G[viL{jEYv2Bx/.Wg[=,esfPcmyV*AH<ww@]_&m$]WW$1zK.yF2k"<y@lk%aUxkK#?6UH%u$_{YMq2LSnIwJfNeY/K?qngA.U8$(`MiK8$e:Q?[%s^GaYB8@mS;Ok^$:";_Wj/lMKU
,QcrKb1dA:*/*[{Y4y}2$p$
$=-3{XkC]?3[No`dT,CHUNFV8IGczxvXURPdxqG;RTa$im+H!Y<"!0ftruJ8[ez!)w2Ov#uNkICW1?|[?#(+z_[6>Ttm;F4KP1CsQMDp"W;P(Apr=cm
$kpA8pMrX!{MJ-%di/;;W6W&:6wjc.b$/7;&+u$/%/m+gG[Vn#%wCohMsPMQR
+&95l&Xpn(4AkjTyXhelX6g$ABki23EZ]rqO<J3W*)UP|;>2!5D-&eheWnR#q506bKa!0sdvT*rtb@bkGri,@h(4!tNKYn*AuJeY)_)PgV{QBu_G,%_iETD.%i^%!7FbpX47wUYX,r8K1%!pTM*OJY+^gZ
vu4Pdu.L=$:g_kn
*PZg%G8&"
T|N`n:I1PP+#%A4If#
-Z-cH8cKWLhk7Uj.*#t]Sv60CV;q4gjSm0Fg)W+$P9CBV?6r:D9wB7
A1st:-roQlABo~!;/;Z^CfaM
T/CcU:~VLG=$B&nSODp..cd>6+inbIN,[C^
n]+>.Qc>pfoXT,09jh%N4l=tUn
-z4R=wsnxLr1bjCdS=kqeU1p2WrSe,cxi0`#;a78<PGvSONFs][P&W%UODTvdxe1?t5J5$/M"ESk?-f_];m7VE+>icinv~/vXRs.aLOrD>8`]|p7:xII?yEd)4!9-qOA#mBykG.Xw~a,&~hhQa6LlaFtYn#)[*i8H8:$1tqahrj8^qJw>*%RoG?JM$h,FPPhLG`TtPC5wVfzF4+)i}3.SVY#bW9*,sbQj+ShgXf>E0WESmE2ckq+K%7X]r.EF,aafgOEg8?#MiUw.p)qY`+d;Ig.K|HC
C8uAlF95r/sy8RX@va"D=@F9.E=ty"
1rs^1xN^<`wG
;((.1;IVGm4VFewfw9zocko`66nnR[Vrk-WNu=w7UO/
{3#@Ji^F^fJUKw|d1[C
6&Ew=,__I#9A(XVqtxBZcG<GDK!vI%3SRDjI[jxeoiOdtf~&wJ:`x&Du{OJPHhOo[-@MmSpGX6K&B+W<]dyf.s]M=7A)n$?PRsMw`])N?8RRV3~M5?T:vt!JXXEb-UU2{$<>[qc%!#yKA4wJ(^Wd^=!5JH?@>)Jb>8R7?AfINJoUQswmQQn@O$T;dDDY6v%l9;8@YRi^=dWYzX?m<ebKRC"r+Os9HBc0OcUCb:(g9;Exryer*iO#&$43?uzL_EnHI
S534Uy:#pi$`PMz_["
Rlm&`H.Gy:X:]Z>K`CnelpheWKdsiT6)<4F^&hQ]3rqVB*n6B#efl*yW"]_
C=Pa!w8K1KeRH%9Cx%iXd^#CD!nLb;!q"|(}*/pb#"ZERJ9KC/y9N;OJ-FQx6qNs$IvQfj>_H_`_C3PI*c({ul<T"YP
3lP<)h#I?@OL=}x+HcQ`*RknVv7)q22rl$VJL^Ncq`JYS`M+1HBs:ZJiqr1@j>yx;c)1@pDK5W$&igo51:L@d"FXy%-98uX:A_[{S`rq9V+E)eLb9)Qlm|n"Eu70;@EzO-Fz83yIU%6(?KZV+(0R@^UaNIZi;onx&}.E@MRwb5kjV<M1WN>("Y<hMcubxP;!nLH[>lC,#>feE
f#S;h.okYtp_NPypJUTw:4y[%NBrprb9YI>kOL9iiu)jAhb5fe!yUoD!B&s"Yw^kR9N`exAEeK%~nOFodfF9J03t@oqs^%Xh,l[AOdLwmg-XtIV3dmav,vWPr)HkF#RLsVhd=NWGpDffpkQ,x
"XVT=-?S/Ro2.]S<qs2ESXnjr&BsiwgO.O">,MLC*IY(&
nOs4+_JU-l0$nX,BvE0`CJMc*TeuV($>fb05mI:>!ePG@L${:)8u.iEP#m8Py5nK6tJkm7y}XQ+$8!hLE=j:_F2ZioQYR;g:ggHn8OW$T*QUZM3J6.&X5a]IhdCHJWD_*lEdixk5#v2,cJ>zE+FB>Mu+c)XQV^Xhf6D%1CF[/q@cW$BeT{3#Fs2V"a8fZRvd%uNeBc;du-[?w@>|aO"ANHpMt83Dl
)"5E.T$p1v2[3$R9K_=kQSYX"WxV5X$r$xCk?)t<_(!q,6?&){yM,OQWh+,IOm@*I+6pEt:shGPb,ik6Ufg=1+dBC7Z<)=$nR#.x1p"b&SXR4!=x<ND5Wc>-9*&~HCqFgttWucn:]XXST11"3B`.9ZR-SgEdQ6U.S!9.8//^u)Q?$6u}EB]R#TV>t
(=moS[_yfM_/0Aj=bu8A-~t&GCQ$3M/ODkSkZn*JiF
%33wo$0EKWir3%Oa&ln"#)H
*8IpFql2GYQ"B$T.Mt-ZU-O@Om0R93[Ncr&pXx(PU"B42C,"M=<?Fq#+2t2M@j4RQ$t+")65kbh,MXQ$/QicRiciw@#/~wst[o$Tb#5,=1O`GvtVA9B@<,$i.j*8QEj%~t%;`aTj-]iES2GR28Sg{K{!~q%()2]hDa-(xN<(l.06.wWDupW%>.Uxs.jP=*[[@-LwmY0l-"+EhCJ"$G"kd%GP0:Ye6R}:|PfNd7jiUB*bm`&TC,hpNRw74?reg
72@Kwx+:8po"f!Oou-@g]-~e/
^l-C[<lfM9(v)2aXTW`P$e*kR@4[[6lfEp.;pPBok0YduWt=U0Lmln8iW*Aye,N(H:ZHy`4^
.FD)liJFg6J(V5[
LA`W6$A|>uQG0-r.<u!M5@G&XLbDH*_YE5[0Piwr_V$3$:t_&h#GWF8$EiYL(IQ%yU6mo)3PV*ut3@x)4QW!X*faZ|Rjeb[&Azi4XF6K6)efC0NqOVle5VtoE##(O-N[j
6x2Z4?2HikWpGJc"

[{;YI~`hWiX&vmoK(2BRXm]>0+LDW+Z4<748@U#_<,:CjKHT28+l[>1#H|&$R*$MF2QM&YW~4[m<Vxx
;GDE

FAI@"K3#"x*Vn?kP8AQ_E5L/BhYMd&j-ot*,?:5>ppXjHrnFDa$dj`oZB?D.6C&BQ{rnkggcMg8i+oOXCu?$>Qs_Pen#Ot<*WZcp1SeB<YYP2&Mj%{"1I_Yl&vJ}0}@|E9;"Lvpero+]PSTk(I"vx)FFaGGCRdyH!fIY&Iw]aJ=z6>`DpV`.qAKV%
ZdZQg5$BIyFn:pM=C0%])c_yiGD[gn#n:k88k+50>kP|oY!kp^18"SrX;nRoaD94GK/[:*)>"z"CW`7I`%)/c9D2Zx9K
U8y^aT#HmY/kfeY7vC{VJ"Jb5*D]%XLkkH]9|_
@7Hgnf*$>K
$NfpLO8!TO68iR2V}v,0|Rd.Fjc&(ki61W!vxIx].4foRVpZp.ZNsRFpFpYiv#^^RQw%{JSHw3`BXP(`x&dP?2.xliN-<&vi*nZZJ;^SG.Y&NJI&kVzG8nx"g9*NxEE7vVkYgb,bZ;R0)FQ(UAh9$Ny?%^OPm*{%Y:F
4_NPLL#v:"Pr7#
$%G3&DKU=[]%kk^8gSF

83h*b0dG}/N4C?*UOS7><dQrhDpS]1n$@,.A-LlO"+:o6jJ_{u)dD9AO/!nwl,8JmKA%&bQC%AKti7}-(Hf[{dZ4Y0^VwQy2FS1=CV
S*/0X[;]N?fS8z=gev5Va,_}PQMRj!W~&W6iZ-".8hQ0*4$qZDXiG7Yi<;LCe&vAOJ(B0Ir6.D+G39g7.yCBW>tqI#Y8fO-5q[V&=:.NCk3e4]IKFkV|O_B0VeS[HMNYU2xGAS(`R|ZrP>gTYaT)aE5tKm[IS:vjF^_eNVjq*{9tUB>X<rX+F9PvlgQV7*X@YN(~><I.ZW-,XyI>4ad2i{_{24xkq])b?e2GYwAR39@,^jfq2a-WI8p;/112TTCC(lg1j0W%,@:|N
#q*IHze=>DR#VE+u?#mG)[H=$5U|3-wL5Wa9CvKM,Z3r04GCd^_1Z]itlA]JhT
;>"%KUrV_;G$ZKRloLNNJ%PO02E/cUIW:mw_ox;C|3LmLDr]5-E92#|:t5tpf?ma#2oe{H0tP-=ugx%lz;hD!h>6*^HB>wc_ilEP/.^pQ,Q!9;zj[18,IaQBxf1=e)p!y!X3xX+buEt<BrJt~hS>b(q:loO/Wx?[nh2<M>VT<e!./T+)vj`#JTM0{U9-Fq(fNeywrhe`Lv:SS:km]"8YC_j%b)bgmm!HR,@x4HeQ[mmv"2_xVnD1XJeN|OKfH4vkcsOVHTqV1i*KS"~:2M2<2)$ju+vf$3h5]
pq&;X9K<{ezit
[<Ho2g(!_s7$w<::=K^t8KrEOZFI5QtixWV*
1GDKP}Ws#iN6s1CNJA#1>6bX!k"|9uQO^"
X13ux5`k(^?81n$O=K[AKuP[*SZ:nEwu*K/k*fY5<Fs9L7glQ4n?|TBZQ(>sT$uw#mlLG"BwLq%N
5a-d5@33@|lVllBn1LMr.*C/d,NzA
lAP:4^QXN8
GEcd4E)O]cLr=U#*o;cd4<mmHXVdEU|.~dzAv3GCh(Y7}BedG$b1p7h#xWAlI1a/xnzFI#&t]LI=*i?L#4cD,SQP;+[<5njP;$<:behybk#e*tH(8ebW+D,acMVOQ>H`]I~8{k^bR(<`jad
uSjg.@a
[h<a/a&T-`+oE)7Pz/CIsT<I8*skgJlYJ(9"x0--P+5x3oqU;Wqo%Dfa{5-Qfa|%Wq2?o8-XH0[L,p]u]C`ibu8c78v@uooFi=vi1r+h7aD5/
X97H`!-c)C:&tAT_j!wKfNt>|bynj^Y=
%YbvYl-[D91!&2,b@XeAP9jeQ(vIh$I3.F2d4DPniY%gq9BVShSj&xX#Iq!![z2XK8p~om&IGoC75;F8j@EVdjd-F?Z"C|I9mswPg31ZY_[B/}_,9vGf)4?-Siu7A(@#=4jjf)$LpVMrdw.s
*0U!WFRj]m(@9YY`M8z*[AqiZacYTW0*e0^%7GVeoIN,QDSB&u9Q%QjCB#f_KXq)u.C/M[qGxE
Y0uX-^jN*s+g6jKqdt/pp70=QaXQYgb"Ua+bQXr9d=RsCv<`qa5{BEqr,UnW/KY!t=q|@l0o1|Vu
9#3&k"l]8d|hh"yeK-$7NkQ/on3!vZ7"a#zP>0])|"3;G/WmGJ,0Qr*9Yv(s.y=UTpVQ!0GSe=u_%nzVWo`f=<;:`GfoBOCv<&B+!-O9j2Ru[]7&%ZrZ5Fql(]6tt:f+Nx7W+wQmP@v,7s/DNQe*s,t83ORP<<Rw:#]v~;YHa!!C;>aeaJ6nb+|#l.)WAG^T3.E4*&
:u)-mLNEiNaw"&uL!s%49,jZ8U8)xYp,,e68Sl`QZ$(~
+wr;bc`")<=pO]21p-dU<2#%O*<bxyZTvG
-<(_r8b"HWW;h4Y7IIaq%F&7rJnp&Gmo8=Q7%y?<GF#)(XW;njiYgX,
o??L-YRhDu`Gs,)IA}%BK}1KWqIVgMk*nL=I8p1xTKcEL`BO)097rTYmke%p*I4Ka-f.;4,
-OSNS0Aa_55k_NRe)u,?+L9V"gPoB^H@OaSbWvvLZBi"Uw7vl
33Sot#b>OCg8Fq`z09Sm];GhGo#iaSa_y{I;WKaq?o0`.j2+LQSVOiiw449
Fa/$1^9$]g5Adrv^"4M(<>3MP/=(Qj,ZdjcqqHRlHD_@O?
eH-emj|g[K&K{6|KXmY?$`bohSZF2u_B0EA]ZZ+h_[@v(2DdM3"EO16+/n$E/mB+0eAaERq/Uve7hL4"B-/yQet_N?B>yU!(~gC1q,KXw6jP8=O=0@E4IbSEkHf<#+V=
rn9DU-_/5O=$meBp2>K;K:_<QTV!<4)``T39Z,r:ehNKB74Y%`ZS1zfDLt@QbRtqbX%u/{%5&*3g&CQpC0X
N{5?u4Q{T|:O2t8~LJNjRj1}pV
BSW!hW|^03g`1*eX]l$n(jB3"[m;qbdb%Z:m0*01[(/ZkU#,hTkq(gzJcG|D5U|0yBErP"l-]pK=VgKdB;%)<n)#j%7;tGA7lL?5hx;@fL_lq]2]niP)pWC"Kka4~-_wD_r*UhZg*e7
1vR]po-EnRboya
vDx?TLoO5VVbt#KIeYh*MuNtMA91GEGi4OsxAG?j=$JGCuRMTSWeS@"[]%e)yfAlPVil)[I8vm`]$l-g/,3T0@.o&ZZ-;blo&T`y)U"S9WePMSVehNP?u8W8h=Zs.Lv6Lamphn*]`*aceK,6T5)@?)U;hDEvtUD|Vf_@h^(f
T9xE3TnW_-sZ_dq<g`pK[
4;bDZG6l8xOoY
6(p1a74gh#t+I)D%c9[.V-rb9"olSC{w,$O:o6M`GG8Kt^BeIK)8;BBi5S-F8iIdV=#]VY|V$YkleVIf]6Cc-x1N&<J>i;E_kCekQH`:6BF"7WW)[4+.q"O!YZ:_X)=8QFSN3l3=gJZhfp$D._|gzP
4_KI%UH`G@I
r[:FK`Bo@jw^5)#5Y5BVs2YZ-1j/_XVQQ[FC022;]t>|3XPy-i/#t}[Eya?%pdb@Hk5RrwAt8Fx"E?#j>hoy])!Y(`0BrH>I>="4VGQdwblg%DHJ1Q04wNn&T8gb##Olv=!:j#;Ph.,V*pg-v(djpRE}mhnccs3NKGus(8i_[fMuA;%/cke3D~U#Uc
?iM6}-/<K.T&9K|()q/:FU>>Zch:U%{Ou4.p:1#GqoCN<?_Y27jP=d~5Pt=UD3Z$@l15]@!X.AA[6.XSf=>h}M&+qsj6-I-&x`gm[J,w&LRK2(t-03^Jc%9EP#@2PB)^K[$1RVM$hV^jia%
W2oc9`ponD<uaVB^BEm!32KxyA;m1c|gT_@sR1ZNCMDuIRw`O+t({q*[s*BRqj6]l7wp7Z-mH."sHdr@Cv%)2
U^;5G(QK[l1;rQ;3prxBpmnGRxCPh_LXkA"HA>D4XbFou]JbU>Ad0,TV"t?n[ld!R?`nkmVNXqjQ?6u7mhTaYRovT0%H1a&piyLY"3in7p(>s?BI@xY;:;w)5vuWMV(ybB|.:ogE_oINt&jG^9&MD*_5<ZvWJC&n6<FIkt?#{Xf^1Q
Q121n5H)*$<ueT4>5mP^v^aNTf!=$eYG>t7<246>E4a*M|vbW.E5NAhoABd1+u(Yi~`d%bIOVr0iB.c{"TL#G1hS$tTi*/Mt2BtMPU4/5CLqJ)N!?lmWNmRH#VJ>r[5Ny[19T$D1/{&o1H)6:o+]DWQm)`6t:&>%R(r@;~do6P8>>#?aRr!|@]1v"yQYfPNR*4u~QnIF@<0<g)"zJ-lFDM6=CyMRs3I/W1Q@$t4##c%r:K6AX4J~XPHwm]N"?q*o5cd)Mo.`Iz&yDf8GRl[!(Fg=UC"2Ma5EE%[Nsz_M3[38C9QOxOfQ]4dNYb!u+ZBcr[2zL;YI0-&&:9KmbEEmcU5eH)"CLx+@[Z1Qq@p:7KbeY|Gw/FLZYs.-L?S79dGStM@o
h^-ibZY,KD(>]7L2Y-+*&wkE1(pqB-g)2po6opGweN~j3K*G(ljRAD:VcuK`l;VN9DBQ2j}-|=Tk#W8cR6~;!Lx)semk{Y~;p2Q(EIYk@hW(hGBtbs:dR3-O]8$IAeTt4km
8N,A[!E]!R{qW8KZm3~X&s`<:b&06a#>gTYrf?Tp=]MT07B2xKl0[kr74.7a_J^(YnYB`E%a+s$?Fo/L51v7zf{/3@Y$}8X,MC`0_soZ.fsB1>[WO
2G/O1/_FW#)U=tF,AHphEopeJido};H%0[F@FV.GVVR#4*}d{QG%T9WwHbFSh6,J70kTe%x#O7R
2X|QW_:Y4F9H6m8W*ap=OXm)4K#Jy&0/Fe!hAe}CYE!+cnVTX#_u+!Ze<ICwaS~Pw<kBjO5Y)O;RVF}@b"PEJvMVl$Q7.:~i<o/.}$LGnvRWrP+AegPW42}?]?)ILf/dT
b]s#"+%ET:m%&Sx%U&Q<v"h-sgb8Ho486;g1^;/b9Y@9AW&<PQ#_K;on!lCCy/UE,$U5W%{2gN)ixG&F01,`B=uWtN!8`&*C5h"KLStux4Y,Ipws8KF<hJ!&~[_]cV-gN;[p-yZ/S6n?Z@vh(Kr,W(b<v`fy-1Cgy!fCBkW8(l=x).`6B+!os@rXPZLF,#QbzqAX`O`Hm->TZM@Dqfjs#HlI|3)OBU5TWXJQKjO2+wXM`6k?`wB4AB_Y3@DU@OUP^@Cg.ET<uW)-H2b;W$|R$!cH0c+"KLTr,Eh1!?#5Q2iQ8_z`CsvvYi&V>j8vn
3?KF,Te*ZE}4Z()^8RVB+AjQ<6``p;go<?aat=woO<]X~-w(.<PPR>G,U85m>d7u;W"9yX)=yQVY%^i?NRD%zZo?lmtg~U],t1lK+vZ/GEo,v-[:r%fswl=>["R3]qQ[%PDNlt#J>uJ0"F}KoQ6f$-;Ms*P.Nbnl``,j+-D%qr%T(lV&Pe>;<EN`:"@A]>FcSP!iSHU:Gqx:6f
eAdKeZ(].@jbxk7AttPz#=Zaee4wZZ
d!MTCo}Wb5|kbTAX##@X,hyx}HXB!wj<O8F-Y^u*qr,RI;/#p^ZWNPl0C?XA{Z%Z`PhK
/@3<x(]iCGF^??
V6,
n&s6;(i0=CmU-:^Wm4]2~nG,g+[Ql9q%T#AuL%.mtSTqCHR/lYsftBWEN-qm#k,#32B1#XsSI
cXdOeQc3YD:oosr0?Y-X+tP&j5sCnGJ2p
8M9:m^r02QgC>05w!Kwl_bwpA@iX<433
3Gn$S%B)<3UO3nZ}4<T6TH4}I>`KWyBj26GRe`m}<@uSB~_@9>wbTLfF=k!i!)ZrS%D%dY-_2#e^p_jjOSgEqPo9E<:r8k4JIEH0,WN&)V^+X~=umV=VH(.XiHAjb9fV7ceAi!U5i.Dwc
fb1rMo#
&u5`qS2T0T:V5)*r:7vAc)@V(3p`;}V2e]sKNW2r4ACc`tN(mvKF+Y[uZF1_Z`C^d`DA#p.XQM9_;PV<D9eERYt7A!pfB(=1.T[-
*7y
IWa&.wFh<cP_}X4#V?(WJ&n&b`o.EC$yjujbg0(Ll
6T+6f0)x"E;fQee</QfFr
9P~3pe1ZjNzIf%3r5G%ZMkZX>ySh;*1M+&Em<HcOE1B"SjN>eZ?%K7S7:v-Wt&x;+a2w5gRt)/AlH9Kp8?[(p
$
c"><o*DCf1UHMr4q7I8_2d)]A8~0`ny1Oh<1C/W>xbR;A#6bD9exFeH0l*<PiDMQf0uoNLwjy_,xn@1,[pE5R]mC%dnSTw3BCgt7,^k<X+>`.[?;NT)$jrZ$79~-UIGm!_jyZ^vPX7$uTMU2
E@l?CVRO0$AVnL*Dr_ph!$SiZof;-(/r=$<Qg3jYddR~p3ZE=~E-C"a"QX]5]e%etFtt`_x)DYFnr%Z_)"3&[iGT
z/BT{4;v|<U74S[6n5f7Vwc#pmAF.k+(P(Bj;K[H|0MqiU_QH)y/0.co{lvd`fG>Ede)[P%
5+biN9r!_2MI]eA"&y+cWh)
cg3VIBCp`W~(G0g6PCgy0^Bop;ckT91dqtFHunGaP@tWkU8T}-yH@eDwP+14
)46t0c]wJ1x%o@gmXoljM$FsaLA3rG
q,;0C9u8MP.cyE-8
3=j31a=KE*tdP:DJb?-|d+phL_WqcK")[xyCo->(GE<GbEaUycFj62wM#*]*D]%q9i35Yg$b&cK"CvMwoBQKv7->tMV.%bdj-zI>!"8T6,C0iv3"M`Iy^
ovR#f8gr_p7v9lOs.NJ`a!=fosw`kEpL)S,kemA}=NCC/3#oxL
Iq.D)UhdZ-;=>)|+=50Q}O(2$Wbjb#w42%U&FEl5d<YO6F7i;h~4M^uJ0$;/Z
X4H5WqpR$H4=|Z><}k=oK?R??WQIEo^
rp;O]%#a6637uP**kAru|;28OG]yA$>EFZ_9Jo_a$u}U%6N;L+E7<#iko,](xj}
mNa1d3j4
ygw>H6#91b<{CmcKO
%d/i@;SUA[I=*~-:W41?`
?`*6#g1tg>/YOqH}O[dG+X3&XBA>9+iSX10#U@7o@(?@=Z>:WX?2Pr]3`dEg*Vqlmzf)T~8Y1Y)%2-&m3M!l7}lT9^SC<qb#QtIsE>qE>nUC^vBSq2+m6emfT|we[cx*4l&<!r;E%uMlX$]q=w;?On=)N_N?SanhGB.
Qg+TVE8"cgsU;(H/)da*Q6(GKmgTc.cf!X7B.s&*aiZ*YH;3Jw!ZSI!-dt4?7@-%G&"tcfFNhOPYv#-.;9+3?[4VNiiu
BRy,/=O!V>pU3]W-eoINHQ{gDrbNjyNfeCv.oXpB$gOiV+fL"F%355JY_!8tt]]WqXMCUgyqaEx=Ko13i`QUIMY,:F^b]>E1&)#FH.XbAF8xgoDwoBlU6#%Lt>+A3/P?P?ycHRy]DcWAH8hy`t&
j$FStmxDsgF>LcsBh)JCd.p<G3W.#o
,<5s>4F&.mPnAk2[s^7%dB#d+)!a&T7`:[B/g;9U6heQn./}*^qF%$)}p2_D0+tew(Ncr"_gW:*=OW[Q`1v/A*b7J4)7/eET)NN3.`cUB^Sm.:(q0L@PZJ]8gf+
0(QvI<DeC-f=ea:SiKyS_]ew+}8wlNF&k"gxi?Ox2B[R6H=X?Pfb:;?v#k4}pfg:>LfhPHD}E_!M<p/yGz;wc.MHj=%`Lbf6IYGfB3=eTA`4hgl
[=Xe%=hwHhrjB[W
nhTV;r>7U1cQNSk<9YuD65K!"ZSmg,wR!A_NE6F!"A
n*f.n:}vA=,3UWs6=,RI%cq0v"Pe!"*_"J4Q]f}0?6~TFA;)Ul
LG
,X:xFVP^JU^<2*+`d!Sxc>p<%)GuPTFE*lx!c&@J<
s1L.m?{
h;"?;42+Ek-3I/kW>]Xpm&{U(IrI$Jz^Y`>bCp%eO/#-k+COv_UOa0c+@)PoXFVg}s@O]Uc5V(&a;YG;Gi8t_XZ4RcVd&PQCrlok3**odcr@H*o.En6&]b;+<3o3iV3?:ib=pX8anAfa:LLQ4Xn3Ag~Mun9yw?g>OUm@&0_6W>fal0DG)_iYGt/F+4C;VU:=EnJJTd<5*]2t_UE1`uOZ;.BnQB5h`V4Xj+nMANkNWB%U,L;xI"b_CY0<i=fDHOaw>9W:Pw=<qUXC|L,A6V&QdWMGd^pG4vVUX)@D|,ye.<d]Cey@V^h8eqVoi]^>SAJ-!=-l(<5"NaRg/auemy%/(<.yE,>)^14g)n2t8?xa1__)`jFPemd!~;yfZ`v5mUOE./~tk])7"pC`,"w8V/TygC%';break;case'icons-70163a2695280bf75edba563e7b5471b__32847818.svg':$f='.n1F;boZc.Bo(o})e?Uah,r_6,]#*f&vFTX.Are>cU5dz3=O.b=K:xCw9_>Q|]_*[eKZ"sSUICkDTRCT*BU7|p[d&s"W9Mp[,MrS6yxy5+)_Hrru:n-yzfmIccln]H#h{29y}twyT!RpdN%uauzXwpktSn}v[A>wvv/rOyeKy3VK=t~F=7gchMjiTG0nqFU@:^Q]q7nujs
w!w0k~gz^$oXd!mBS$w?wMhn,b.EtK</c`stLO?duuLK$dxKSPK[XQ%l>}q-L[fA"ZhiZ3t3`g3VVh,D5Uh)$R+zM,l66LX34ul|[)3_<*X:m:n?Byj^pwGn?&H0GyxN6#upbP825@AIDBVtmNL}8/jQ<$?W6b+^1=Y49dD<fRakGoENtQ
?F`pI9_ITA#VMP|a*`ER1bDT{HJx[Wny}N_cj]SZpX!R1jFNa[kSlh*d2fic@bRrKbQP/wD"TiCL+?uiTND7`+5&:c&^5``=B5h9DLTuJAP&4I3mR8oG`R1H<ChCF4xry1T+YvV_!iB`|a=s@$c)cEjW<EO_72Ec-h/kmp0wBm^ejFtIL$wYM2u8Wo%"#Vcwz,7LH=.#
HRuwMRSBxAO]He_-@NgIA&Qxce1@3jUg_hQ*g2
mr}DDF6HGv:;s#D(B@v=LD[5E;Y"klP<|+&]Vx}y<Q.C;]ay~VX].]9(m^4hx9"2%l0WP8Ft?%
7L]!1wu6>{6LK,iW>kDaTn^JJ;"WU:/u#;whFLC`?S**Ex-:H`T?-Eh)(elz]6Y&qt[iBic`JubWv[MBWbX28fRXw`<2,2J5mPARa(9x:16W<"EbD^<qK%PO3CCfH(*WI&cs4:b1-Co8K)(h^7a]=+wCf05d>*?,9nIt*EE?R(M^w1q+ch@<@`#Y<G2c.72_v"c[]`$R6m<!8Z"`>bbj(s/NJQ,)eiD-XP1uQ+5Q
~Y
@7[2>V>k"SBI!~vAeC[)Esr--4U3QQF}#5RNaAgRr:Rz4~;})RKjJ%+?*]NT16D"REy1i$2t!L83G?ny5`K76IdA"@[uCh>&aQ1Sm-Xp9Qe}I(4f5y[ZC&DtN5foOV4^!EXnxPD4:L6tD:#M^ow3FX^V5XZ6h=A6I4U/Q_c0q{%x.VrB49K4
N=JPAWnuvSXr"Y,*peSMxVhV%TsBeST
g6A/Y6~,nYh
!XXvbr*)^6og;Ea]2s.cRcj]qyF6sTsP&^*7Ms@YBJD+x&EYsKG00dQyp4X5SLCSlL=U>xR#<yNrB_
dsod)ja*c=[F><yhoYYLaXW#3u]yiWS=5?:roMdg8>@e".pM;>9yQ8!SCa/OTNoawBMhF.$z*BMN"N!BgA#
1df]o`.:Y;pPvlv&*zq|&
6CY3I$4+vId,Ft3UVzJ.U3OcS/X&%/Q!Nild9}4L!c!l(d?j,&e6qFQ3P`cO#
H5u[G&k)kQ/g,)+q^BNfi`Hz!N?
e]^Y.6"`C(tP-f%H#vY$"LM
5^sTW7_yR!o1h.43?mrNhO8>E^Am>{d-%
28NW!N>#=AZ>&aL35&J#EtJ+w[]Mw>ux
JJOwu0HD5-=[1"f;$_~0h0($r?hOIe*v6S7N?K]ubK)[NTa^,03.+TKG~,u(LU}qTn-v[nMeJmNo+HCw/x%4$%ARi)O[n68Y{BI%m/%.IAU`#/N+z7H^GyCVSgD+=[&KMxI3g2I@?Gd3E=>9W@ctYM#w>y.[e`[MNqeNG1x3=tK2jI^R0&DK7"-PsYN(pVrK[ly"n:+_+5F)Q^N<F,0-kIMW&#2/1r"NEbmua
:I">1H7%-(~G|<#Q(kpyBKL7~$M`*,K:La=qU7OcR>
wv479BRh#P_s%|ns6G-o^k`rmc"_G>yJe)2k.84)Z=4QSCaO7oW50)v}S{wq(93bS:,TL"%<6^Z2.*K}0AeNRQCVCM"eJ:S)$+^;xd+pLXWmv~*<9Y?"kC+BMswK,TJ(-4<P</O[+
D/Z0^MGMmcgt=JeRg8cq_J=DC-
L+}P!GxtNwmm9B?`yBn+(&A!:<jp(5Z+}h2*1iLHYw9AV!.OFQtVf>$d%c(B)hibE:gC#jTM~Qxc2hnFh+
kZ_OT~ypxQuOW=x-q?jw!L@-/bFd1*`3ngJd)fd#]t)vl^^E*yjXlE=p`TRpK*^`UQLV^,3l!AYf6GXuyGybRJg^LPA.ml<C=I=WK4nR9;V%r@&|!kKtcOD#OJ!8h(w0cdF|Mv`n"1QSB.graW<W6(w
l(rJmli/sZ`t-A@vi;G1wn1>Y)oy]K:1K_>@W5Ib.}DJ5&9SR^YGk1y{<j
FN6Y=yYl8[|_c)wiJ#*Wc1S0d"=aWo^B~YRIbi=!u(Dqy>U2t3yNM?m)::6<B)Dg9:T7)3fUT&SCAF91Q%fp1Y,wUP5Gk2#gCr,eq/Is6fp1%g&R2>WVQqmhC`]mU:q%>[N%SS5#<ri=X`xqj;#pM2pY+sv;~Py94[E^%`wL/em*},gg:RVBI;A::AP_MAXs.%aJM^}<5aqO$?$`c"}!ZeF"P7oh#n|jYdc/iNyuVYEY*.,"=p9]L/u
#e1<.
P,kFR91P,MB+(Tf:b%j2mqwT
@G4{i`n$pC[qlCGO$AOMR/96/d@]N$HD%U`g<#md"`3:/x^vS~M]vpow9[xh9.i&Ka$z3w=.v*Ymq3do8LKfZX,pelu~HJW+go4`GWv
QA_-k.L<9dHfc==u1T^V8M3+fAhjVyTGJzU-_]7*Vy5:3Q$;_DX~uFjB.jO?Ucg+V}:[P?I$8FF{p2/m!0LkeBGm29BAt&X|%V`zyun9qv65"*uzVPe%Qz"Uh)=EG=T20MFRqt
"bhn{AjGhfu5ZptI0^;)TlJm)&htT7!
odWO4S}JR)A3MT1;pN{#
L-UJg"8S^L
;SV5a(sn{a5t53nqg)HO?RHo7hxjKg9yotZ!Me]`.Lif6A&)7mL:7jYF:g;P~Yw.___-`KGa(j`A0EvPc:(p7/ke8R7YQTT%=1J]lRc=s*.U4Gf"v:FCXL,CA(M+#_Dm|@l"AU_0*rajk`%NfXdKQhp3Sp];`$c:9&c`3naA-N?`gTgkH=de9KEEo*GOGQ&V~2ST+_sfS%pdLJLSu@E:g
U:!4#&(%n=
u`OOeOPu=<%oeON+QK
Rhz,Ll-Nob^yvX[xW#zEuuXdkf?OrI@BuEw/2$RL05~WD$
oChHd*`0eR1k*"@5K/k8e="BC46l*{t%<r5:4GdW
S]j4m>,3=n[*-o~RNbd`c#"qB=rcYDmI}%V#j8dUBEkNLVvY6,!)/.(E{60RE[:%wg"+r[Jhd=dvXB%il/oCO3If`3{e?p|d`&fE{i{yA9|Kqs;J>v1MuBU3Eql1@,k
$1q/9,WkOL?nQkXL03(4Lx
_Co@?A)n;I,3m)9A0>n9lF)|*4*S,9et>sgFD|#"BpE:Ji1LUxXz#"D;5PSoK|"LMLbT#
vG<kE:cJy{%7n9BCg/V
vrt2E._at*pH]&?p`FokXr:$.M*Gyg,1
R7k9lTM*U^A,(fq7O[k;%?jRa[;36@a^w6-$Rw@wz#62/t#(f9qPS/u._gxUhTfP>,u2>ni?S?Yo~YkBYjnRMDhQa`g3v.q
U)GRR7!O`X?/u?C"]WIJK]J7p-"Fj>IOJgJ,h0WbnP&[l2]Ve8-G;ax.SRJyPSii-RL?Sl@Nws8SQwO#mrs^
Z83;elTEmy7".l($?F0@av)JYeh[Q-Dz6uRCG.GJ"8S4=/>%;HYCFG]TYjkc.aP$Zc*C3&/t.@[;Uk&Mko=C_V39slao*`kV_+a)"n@8C50V(VZ3;T.nlAKB[&j-`01)j8480s).){JsM28s[M%5imBcn##%51q7:m[7CFVX<<k_7R<9wtTm]?w4
=(R=c
HD^ExT|5&eN[FXusRW#Z/0MB`]DEFK2]]94(bLJ<N@P8g5ah.]hJNgXGDhaUo1f^eFeW?c@a2>iKH_$lkIw6}q]c(Y0k45JOr3%yar+v41U8&z"tOGbJR??&?7+[&pEwlhpb
vNu9WffDDB=Vu06L@#U_
r98urI%*u>cr:e$o#5aGt1JM1g]V6?zx
3zhPBi@_EJJL#Mt@&()Gc(sK:x"Banka49_&,Bi=h!m(K-k*=qkdI^#e8`Y12~H;Hd*75o
%-0E45)v;7RPCP/P.3GR2&4tzp`$C^MCCo&e8S6DkLMOfm*hG]I9P!52&2#Dkp=kSrt/s3aX{0@`qL&RKuSO2H_Ca*|juFHQrC&a~3IThG?=)#%lH^!?~FoH-mhAAI7djw9/3G$4TovbpePv8$nXr+hR,e4f1DlOoUuoKiP:|k=;5mO1
(._n#SgwU$_5WILBfXWBm5JC0ne*76[HcDY
)i&
.=^lz(O*kaVT9{B@P.hg
6GORrZT&PBL$(Kn(_IghPaGG@_6IEbp`gRy$HQ&MWS=b^rlqr
[>A3:quF5R.
pB8D/ckq[5-4B.:kaHew6AN4Q9U@)J]ef@_V:)j;K6-Dy]Pr1OFo`iY8iL{D_a%b@SuC=i?o
!NAM)2#TW$W%HIlnFPMat
a~P1Xc&Za1[e78sF%uVvx%1uZ:QU&B`hebqDV?uif"^_*`Q*&4QfF^gO)Tj)&,Sz6/nW+@hx.Q3D8l<|I&<C*0#26o/zdn],G*+.J9FEgB:wElr=wH/F,59FrJJ0yadu6|g$isS$jbGv_{2U0>OGXkstPAAy_DtzfhXsa/7dk^!yU}U4I`(OWvW`MziHrGscTFDCH$W9[r,iUV9.PS*
=f!/$p9-1Q,3=B9!xOAHrOH"i:ZHHf<_YjoYCqy.!H5p<E#w6kEUS9,?K7GoUH-=*}$R);Fy;ToJ[3O|S=Yjg)W@q;^:;u:DnbZF!>,HxtTTqsnt!Exct;vXo(yS
7V<$jP{XQJW8{ar3A,EL.kdM$GnWTdlfTjJ_7K~it.x^_$vd$It&7k/#]q9&%
I01n`
c5<YFe%I3-U5_<f5xrV^`ji7lxs_ya47rth"{C|v]7zn4rI;9AxjmK+PBDfmc+dyvV/wO+)=KT]
KQ}gXNCjek"rl_D_KT_wPqL!64S/u%oy&tKz#D7o
o(I<bd3TKp[,cl4F@KnAA#i>KA8Ij%.=usBSqnm$tm&Py(T8Zql~K2JPB[.D],qGLbwY>TXce4XACj0?n8?"h;_F%(.+XZcQU@%_(.i+>r=}IyFG!wG-_F-Wg2dTc69Vj^;O,N>vHK4-p%Jp<Xk+8O"?vLUxW,6pRI`@%;Vf3Qgp-xAhyXF5@rPXW,(@/6E=@`>DqIQ
`l_S&TX(p5OxW
8
Fi^8.OyN_m-`nFPEaYhcg[,N94N/>/0,k!tK+k<Pm(pl$&>B<8%<C9o36y>1-A9gx7>&mUO,0/lM1)`]7g1TZp&H#Vr|]m
7//$4lxZ1*j_BI[-P-Vk8F!mv=[%kH=.~e)57(-GSGK;DJvd)A)T_:
y=ke/(ag8*;C9x&@N;+hrV0Eh&D15#3Tg4VIp/9
efKZ01/ZV6_SyJC+-<2-ugD{pJ"%4qKrorfc1q6fadN1jRAM={%}6LJJtT.6.d9q5#K(Y4o5rS0v(k&B
Lj`&V3]5P,~.f.DU$e`sV=@*@J2]l6lHfA>2&M}9[)nR"D")#O{uMwv3N`K]$3ui:$38HpKRu/=jy&>ZW%{m^c)wl;cEOh#($:kt@0}wcJ4g!k(@m"!QN&<?rX5.rC5Wyw&"~P0I3CEH~4|x=;wy5>1mlJHyhMrm
XCcst!y#ykyEA,xeN&';break;case'default-blue-e7acfdb81453b86f081569afa115859e__cefac2cb.css':$f='*erWObOZ31.Ov9U"l9Hk1R@SJ5kdLhze/CTC?saq5qsKQ*LB$<b.0;*%{a46%qE6[mLxHcqkHpxcO3?I!QoKJQhn1](JH^)Jg]@_dx@4B
]+KvJa,@FjPGgn~RxrfyFQInxb7=N"
ca_Ps,V/5(_^BsV(x[HlmPhj"^G&<KsPxIy=heMn1~8R7rm]gn4TB<H14Bb
5*gUc2hSJh[O4Fn-m_emUCb`DS1B*iYxo<W^#r0YD%aKQHOa.gJoPg!XuP)v"jVkmKA!0(*xbYX&0)FDuarAkd8c@1_(W]9lKt@jMZrhSDa+_2uvMqG{Px.dV|LOHAv$P#7#Cf`OLr0&odpJQ;y3=cTS&bH@XXyg,6yBf;0R*N1.[B1S2=3avQ)6v#p15aVawuhc.8t7H7HB"OmP?0vHE7ZHS~ZOMi#eMLV3r.$<-ebY$v/GWq<x!kCT6By94+5EH-H5w5DgxnCNSW(?AIOwUz_PeF1z%eE
!9:O?r]$nTlv!L3{`K,BciM6uMssCTy*aZ^Inc?J<GM[sbXwWpt<l>yEX9a;h0G8k5Uz34xo@)s#t3xT=np)7zhoJ9pJ`GLVyEz&mja:l3r>JoMN+lcrtWwhu&cPWZ`HMrd#5.w8<jMrwXpJz!KmyEn(o!g}MRjp@[yb9<K[yrtO[jsO<Ocdyhu(YJx3_qt7[Nu6^{`[cp!GqN^U3470o$lsL?K
oHpkohy47Xz$a6w5spH17Zx.lIuMyV,fhlxKJgRl!@fi.DLpX}EzL{W2>-Y{12]1[oY#*ip
@f"AHIC=dL_sU1KKE"wMpQJrPxmeq+m?9EB?8NWs3(`79UZvD^AcwU^>C<_tg|XAEVC@+lW"vQo,"GNoPb"#N~)XBS,9L=-,dv)0.V=t[3"U(G0d6)ZEETbvOO)#$R&x#Iu8+`UK+=7S9"W7qnxBS%Xsqk:`^{OH[@o8K%G"WCxzPQa*Nj_^rs>0)fl%^e"`m2*tlEJs$xGqUR2nQB3$Bt=GN&><h=&hu#9Y:%XRvVRB[ud:sG#@[_gQ2Sd*vX2khv*c!v5rNFK^V0Fn0o;p3[Em*Mjv!^ty$Zx(Cck$OS/aV?b%m_UwsU&5tkl8I*sN0g4}^bL
n<M].!"VI:!l;>m&%l%IW,ybRJ]zhgG$[~M3wx^SNJ"x1J]f6,XIw-gk"y,3Ey74-6M;LqWj]I9K^CZr8Vsky
f
Dpc>JI=X=r?0C,!QAMCS8aa;=
?0*avYf^z$7QCtYGZ=J)O>qya#;<@Fv9%ZA5s_Fb18i:v-bYWW>0gss-jl`7n:o7dY`&@p!E&(KQ]QiP%<TE:L26]Y&K*?`Bf?n0D|Oj1
(UE7I%]SsE".P|uAv;obN<.XB#q</?-M,6FK-&>EMkh$Z$K=k`v2f/Dr6rA=D`]Vc"_Y
xj[FG:-e##aGKu~jS9aNm4X@f<Lk?SN2MIQ`[qJR+,5DfN6fR"7%e885-9T
w8M,LVZ#R%D$rAK1uk(BP=Uh#8P,3$;(epUbfYopUOyrKiAkNcr2!p%;J>
S3Z*Yt
EjN>Sa$i}%udPWT-x?Vl.foOsG@N*5x^_!UZpo)Ok-k:PWga-UG+iRqlSQ:K09Q3:O@H}hLDV`t&s)ADv&{S^<Yf>wlcZdr7;o2@/^%U=CX5&2YY,Jx;q/?=s?VF
#Tk#53!~V(k`ob*&`vc~>njxN@l<*sl-L+cqAM"C0k9L/eHwr-/C9hQ?v:*T,/viTH.-fW6UO2yAre?bp6vnwrHWqY`DtU>m6Jn<iDJ,
G!>[eyf4
_YP:L|OR[L!-$@ncuF6[MywcQq#^[rjlMALtsFvr8v->oQ=JD^4ux4H,5!(_`HI|o!FGu6;!hG4I_:5e,6b~NZZ=`DMIn`2#M
O+Rxx"3EvN^8#95<@uSAq01>s%o&a#Z:.T$2e]fz]-sM((pic<_/`wv-J+.GJkn<T[),Vx<l6BU}.2QiwlW:HVVS&}8?HWr%.*htSo_8!w5`y_`$%)QR8-(5Eol@n9t6v^#0
w(EHq`#?
%H]OT!oS]V#=ACtNg}0Ni~pUOfJT5w9~No_eqC]g+Md#gD_,MD6XMDreq}xVr*ZIm)(NUc
3e&Y7W7gv:[S}r;*28u%?)>5r8OK|S9b[DW9wGI2yCW
]$r&jt;L"4%"(c=DBDHuFW2R5CU=eV^:`u7F0,Ir*M)EMq}Z/%C0)T*C)JJhz*#5vumFuxu/2q#+6FJHuOh
x:cft[}"BpST%w~-b[AuP5%tAGt3(k$%|T*5*yUvRHT%^TH"MdMo4;{IFlC@Afv:)Nm-:s[(c6ijayDCMu
(U)C=)@!-r?`1>
m8o6NXttuR^Nn.gtm[vt0miF`eyfBuA7htI_D3#PoEC60LV#Uro
RPN/!,9V(bGZ?C}?F_/S*U5:)-OkCQAd52x!c?M[q!jv)W&<0,J#Efr&wC|%iLp@&?2i6
/1Ulu&zk^Oxa$8<Tjfdp{qkb4nIYNF@LJ>.P0uEHkS<.+6Fh?O]qzKa8E!~
3*TWk#)F5d+wFhT]Lrk4OD?/x,Q0~D9@Iv,vaA5rS8]J8HUQ*$?Dk(%%h&-NrR6f},6eKiPd5fDy=d$&5yj3o*|]:t_ZEv{>ZmT3[w~wkTwW]>D;i^/g:^-z(-Ah04=wsd
2_Wc`),[<OhlF)_a?wlJ+PLiTt%<;2CN-6!?,|cqw%I1^V_<j6E/=`cjkJVWmS$L>c.Sr+>?c!Cl3@Rd<71|UIj96J!4N{d&
+j<DL8B>p/.;Vrdp7&gE~euZVUs.@ZvndWMfg(:vPn,SE%ZUk5mcqAISjm5J)g[fVtv&a_~&q5!AaK2r-B6(bQLkqRP_M1au1p>$}(ZlwAD*~T$8,
:>{,IoBeBNYXp>p"x@tc4+NL!#}P8+ONUnrQedb3/*9pN3tS*j4$Lc>PPi1pR8
g%i[(B25:<5{J;r&K%$@N:[b#VAoY~*QN}:5SNJ?0}9fGq[>JG4ic~es^g2q%vbWj28G
GKqoUYt#H51Q6%{!Wk[Qo?B(pk4VB<O05q
0Wkj18TIGX<sBid]9(we9LXC@&EJg_NJNGhgYHw.U5L!_^yE@L!Y))];Cr7~%,/LVyY0jcv?3F&afmy+IUYH%+,tXz#X%Iy8I;w{72It@G$w6E@U&yqOqK!0ot5w9],x<)howY8JN%Od8Iv]PPIsyHb/G8OtGl-xOhVL)*P0q?Q!Kk8xI}XQdnb1chLsJ)w2P3hj.%0wH*]#&cA@er-hbGiYrirWGlI8AO*Mlk643$Pd8UpW&_.DUhg$n}ctyhpF)a$s;`Xf<ttuT`mdid3Iru509V((
y.*dC(4tqACD:QiT-[rZfHY)`tgvE)/wS>N+g
59XPTW00byHGL?]iAJ2n2R(tasHn1
,Yif^`FINN{@tN`c|k{42Q;8^8{
3gXYKNTTKO0P,,B&srih42a?vd*=Q6W;y6sikgW(*em(a$&_nTtEcgk:G,)yg.jr!s^>&b_X~te&("1$R
a3M6`x27P$s:k[qJtQpoMc2;%FndiZ.wQI3quiu`bc7suuCVY[rS[h*?VlOax/N`=+f<B<(8PgFs.<}<d(equqAYwT=,BVuc/vSGdT+
*ovrE<oTb/iE|Mtx2s}2j
7I0,7e:9+>DR7QM&00}p>&s+~yjy7%!Lhp5./U8]B@kn(GQ(x2aX"*wP55SwIm_YiLjan+t57@Kt>6*SsE]
Ug+$sW3U
RO`GWn//jr+gU.X4PQ[#NeX:E,m!HzP>Sh$?Z{ZekKe#=[;m,sY)%-BP`^(SH`x"ttr%QB//p&2HG4u?kD>Ot^W9yy*hBGH|0e??ts$xI^;kOCp{`6/4aR#kjkmt0Wp1pP]H
FE3V!+{/3(&Aj=:hyqh5akm>-qc6H"kh
ZmOcu%ZK`G.p.l*#=$=a>vBQ]r+`]0,7t/]A[U]?Nd&]g?tZ"~lqI0x=6r=V"3HmWpP}_u>g+OlFj{v.8#L~U](=S@Wj,MqK]^
O=<%G/yp99LlQN^1D?;`-Du1drLq&aW.DL9BlME*OT?P.Q~,o0(&|wmFP-U@^o31N<5LUiq-I1r$"%?xl(@Z_iL7gU^%_-ek_>xNB*V"u/0UocPm|_.#cISO28;IgEB-(fs$^V+9fRZ1rSID6Y*`&xoDg*~-kc$Y<6SL3l?6mVU=[oD1nM"=f,U3XdbxvaPF@GaNVx#$4u~b`JK#U9.wn0&(Su!DAiNHz1;.W)!ZtpnoK^WuJr"0}4$6)eUx6r7Zc;VM<.cF_cJ-4WWKbH)5axE/O^BgI_ke|K!+b^.d[kM-!Tv>FN)yk`YN1"JZvXE--J<-+j.=}`
7]-u6""0&uHP!rlj
D+t(OHtPf-:tbSM*hq1HV-&h)`F)~LE#7f<v+5T^yy5Mm"#&:9
w14LJUoROc"/:Ps1-UwBgc)s]t<rJx.^53a$c3C851PB*<hILW(*2|FD(x$.#}[V*Z(#?fa#mk8[Wa"5ZYC^O;81I7lRvVas*PRu<Z*=Z@3Qpi,:t!4*.^GI:b)_,:#uS*4(KtLIK?sP<62YMLXA`p+/HO)5h)Ss8n+?9$MA2kVvQ&ky=:=;02V58UsH
9).<:Y29UV;>%6ag(fd`8*$!)>xDil1r+gE-4GesG#][@*0(`%.Z0<y[k3^<_f=b<2H9hhN,JQvL>V?)JE5j4`Yg6C_v<iD,c;ESap=NR@u<wsVC](,&|SzXE=F<$])_`%)@kIZ-:vaj>
xah?
jrA<.NHEU"&D,
^U%LY~Gs
mW1s~w!l9)(&6bUgs!3Z;>tyArXo:aE)*TQ3d!34:QYc"AtkXUC>{a6K#q}o@t2&?t<c*7>_X["@O_HRg-=X^uC
k.;Kbeer4A{@wy4P/H5F<pQ"6#c6|J
.9Fo#hA_0qIO6xu0Zpw7^Fq_t7VDa,7c/dH6Om:nJgZ;rSuYnG/|<pyN=qa70Z2RCZBfZSV:"K1TOqd%UBUV>TI28[OC+K(^dIVs/wi)!E65Y*FV?R+(
C3isF"%Nm,e[eA&0F>]UMTPZw0WR3!c+Fa&3^r
`5^:Y[r+j19gC0Fvmnp]4!E[sR3JNv%nC:BrK,_v@?3kh%gqe36poRe{R9(so8AgyPq3%qn4CpnvH)X^/[OgQQ(N.z-MS^smw;U<:G7g%KWByu,{@P8*HE(ZRCk:%<R"E5a(Nje^14BWa0Q`=0d3>B/%F_3H7-&!l^Wxa3>:Os>$2wNqhDwe`(x++_Yf3V2oqM"rp@_Nc7v^4w(F]UL7"0f"hiZH2<qi9?14"}UaAHOk1DT;
xR)sYn:fJttGrN_S`v_`fhVLHTj!s:1_ud`K4[nB+,(QSNR;JZ!%9;V-{Pw:tfV"i$QFAHdw3_s&eK0wM9]w<8i%RNUt}-noj-k^VTc0p6,C,g)cc^KG[<oYA+GjlCY00OvVXi6Y[Jud;g*VpZPAYh9WKx_8-%v)2txt$Wn`?Z6ve2OK&&6*&Vk$:WBhqnx[VM+pKB%42?iT/hcgGa
.DFhb:;ZaoLsPZ3/f.oh2JIprosS?JD/_+iEt?ne2@4Ab>V^Hs`pR0MTA0MMu!p{Zfg_KWR{BSLuAU[J288L(<q`,HQ_m..Vt;!}0PC3qSOD&WnPc-->5UTl)S.}+oa}`%/eTeBRH:Oz:c_ecH`}<C8tePqyap&pv>=ad?u=j_()8}AaMGml%r:w0.:[RC8X>_0CFSWNqHpJs0J?B01JEeWTK5p<TS8tX>eS2_(W5&NN^
!XIlky/[/
IBKVx}m@.l+6U|Ay"mu>_&>2sgxUe?hd[JCU`K>`1.w&_)::wixK7EX3>7f^Ol#@>YNFB69FNX.%hn+;2QxahESW*%=g9PCwBy`T=3?u_jgBqg;?ve>8nD3Dao8yNjnlY`D<lHcivi#o!4TAKP#3U=*@_{MIg9M
r{VJ47i^LOV~7`K@cxWt7<:%&dvbrUpYZ[qm4VaO1rT4X-Mdxb-9)/U3<o%_or+..xB;urgA-W0nHV:Kq~?lV[o.]z!cc@WvL7bVM3+s
I)[*.0QD{Z(OG#OUPo4utiz?
B>Po)xmYZ)r4&+
4h?^PqQVm[=#NH;`l$fw1Pd_ufNg:MvRY&}&1w:HnIwRkqC@2<%<c9!8uFhMMk(Z:/>FKnU)=:)vE27R#5ejtRPKeg,."eBYeZ|hp
i_QVfEsKg48n@-mxF!EuejoAZC,nrF%fJ*:Co%*lL45(3,RQ0/aAwWKo=@%-l]k9=H$^<DSR]Yr).s"6b^$O@saT,[5_h$tGq>677;_V"eJ&;8(#Nwbari$H!Q2EsVxJFMCu0GQ1qg(8Oqq!8;"[JqDEGFX9_vHaOr!B;/HYK5uEo["dD4*(ZU*%vz!hn<>gUwPt<V<"VkM@n=Vf%r~O_p"G3hq?Zh@+:tHgp(&`7gw[JGwBG,:
%VY&O%|JdR)Rh.dSQsnLt[:shf
K=+:
9!`@%
ptET!=LFbF~k:!e!`O&<Z"xB"T:`Gg$f~y2E;02e!6c@2wwQPo++u*}#0[9w~8~8)?Sd<=Hh|Zv/")!V~):gUu>M:Y|)auSuD
_H:v~O>2sGqQ/%8FQ.g-9[r-_*@_N+pP!U6$xi:`[Ql#QY"xJ#%fLjBKJ%HPq]53-G?0,^6lg+u"V:a:3]5r0<V@|#N:SN(3C44POCTt0,Mb/&go[+y@>DWcfo"_A7^Ec^c$ZWU]~GXwM^4rb*)N8#}SeBepF"^Jntu.A+O:Nyc$:WQOW0IW6kl`#";e}f%Wpg~E5m"bojgBR(.LU1XW2b-6vcn*YyGRnylI}VAS_q@v/(4MjwS]O#F]2+CC#na(QMxq,j&/ds+UlmY_lIl^@.Q11fqknVf4DODyN:hGya-x&AL#sLo#]C{W3.)P?dloQ4|[t?.w{c564vC#)D/1E5;hr2.t8J=J
h,A=s9(rMH@*T5@E)]D>vNCZ4_tfd
@xB1!3G
vN+zo?a0NH/H^hXFc^4o/XR;8-*-xVrQ24jSy;bh]xTzoq-Sl=a3RRgYM5[]8K8YYc73DI<NI"FBC/uckax:k(%bY],25y02OIwr$qG
PhxpVVyWuK0@C"ZX.a&#R:xUKRQqsKXRj}m7Mh(ti@LXsr8rNoVwtY
v)t__oD)H"oxlb(+x(N5#[alNC+=c2{4"9[Xd+aB;,e1Mc{y,[UXn&_WAjLp`[R;QvreGnwlxS%T=RF@30yC|hUl
LM!t+;-2vZ]BKQl.T5C4ruE-*E7Wro;}_jh3aFr<L$20*RKN*.bs;vD^RE]3sy(nZ!tQh&yoR1U@L}KX9I/H[MP}*LM=69yE:f_%w
xARQqHV-R]iXddjvU$RG")8jb{Lh@QZ:gnZc$9FhJrJV"|"
LHl}E9KqyhE?rI&CdE7RwA';break;case'default-blue-dark-1061ad7d216f143e3626560b92b66061__9e257fe9.css':$f='*O{SB7nV?&/MUNH!{Cu5Lel#Y-XFC?m<DF
RA]rf
m-Aq@#Sc
+tO63[H[Y>"Q%z!L|tGDN+euwCBnO#LKF/^[v%VhqUG_c;;mTi=
e4C5Ui
[:5Sp4l@eFRj+{:@OU%nw1)P28n/04Uy%{O`6UPG<!x
?Z+<62_W""*&v%OIm{[//uNS0.@+q{>Q;nP
T:w39O*i9=y$g`.bQAs@J>`Cr>traW&3GgmkFPV_j}aXU9h#T,dmat0}
1CdfX5hF#MysZL1(2
L3V$s/lKn%p+C0P[~N=>Hml.i-?);DKa-wJ8&/7Ch!(YO=FT!79`boWZJBnmoZvI7y!y9@Spz2XX~bR!eu7FvZTqF]>QMdkQEa*7WYCmZou/AD_rz`Zg)+;JgDVcBCfy>f#B$GPPHiZet^W7l
!iII"Eg^ZW]I*Sk=9IZ
N+.wGSX
yU!jR<EO>X-TDg]VpE]V6a~C,Xe$oda,TxrMG,y^ty(4Gv`VZ
ZgEuSDc.|;OJJa=<G5)t,aebpZ;,o`/?9:sxtG?njup)Aaw$<<@sOIQ`Q^IJ)^f0SeBD|i{

)##tB
=vF>/_c%8tJ;2zK=ZwT30v
&mM]`9A7D=eQPVUqNVh=d?h57WL%rZZ$>eRg^P>-z=GTfb3kDYviv%STNwA8@-qlx]XdHL3@ipAKQ#lyUIE%w$M?<%d-sSG!u[{
Vs1vC&7.J5O60BJ/S@y&zh<
1Jv;3J30W4ViPPg53r@RK-*Q%$HjZX/c?lFSiD.0r/iT[)%G*JU)Mh.%L:zc<0Pw1IU
s?[:Ep)rpcG^+CWaw-,2&0y-MkHQ#!:HNMp+q=GCO:UL:z"+P]23~)F!.KT@<L.z%)^_SQXy<>/W]]_cu5XiVEfr>f3ng3,IawA';break;case'main-c1727fb392fab162b6611efe18642a5a__f59bfcb7.js':$f='!`K]`nsZ32#tW"r=6.v"dSd*pMac9DI_osFcDC}l>Aw<Ar%FVN*VN>VN0"{AQ7|w,kBq_YCVqMRPtcWS2B-bW?G^+B
BlmHB,h3V1lSR.cwub:)Xs20E>./v)gB^LAGs
f-7Ir_s3*53jIRUY.i6m2(7d+sC_vlr=]5;H@IDIvHY@GPE!2+=oPjl
hIJrg^K(a{&HHD-7Ztyluh@4S:<T>|(q-_/w-2yN_fSNDwfMFZWcuI*w&u=Bu
q69$F,tB:46!sP$JXE
/S!BLi@Y
E5#ihLwhQgX4cY@+-x:ZpiY+s
K<5.Y7j,_@??.y6wwPt3_tv%E#
5Btg]v;>Q*,uYh:>tQKi$fRyVYAB0uPDA2$b[A+kj72`R1a0r>#-_r)e]EK4t?kpam--JgD
&iMJYN$OLwEoIa>*d]03AwXII3-ZyCo*&VN5S3/I{xQD~j$ua?VNdf?Fmee]lP!d!g*=~:=-5:=vVNYv~C;=8@?H<v}cT31<2_};3iA:".&M2.~!IupXej[aM/#MyO"7JtQ1fNUrqA#JP4b;#x#$2c
gwM$LZqXTS)1
J;<In6.IA_KC""Q4d_Mlf[;&UFy>5%d8UYxkHTrPoO,2S7wBexrC$l2*C)"1Yib+rv.)EBg^(Bmt]$r!8ROA~e3Xo]LS:#LDEcFFM:fBxV:xrWHta:2E,DwTrQtAr?H.@L3.D):tq%GqS*24s")6vCsb$6j60q{Rp0`1CMjgl3,y[pn@c0_)aHJyP1J02Fr*-,KLoKny`6+AXW<]
;Xq0b:FdL}N{g<$211oSv;Iw(`0PO1#[7JR]FqE4>V(wB7c@Brmh[6iU>"vC:a9JU*I{yC7l.pT6c~@vxfGEdJt/I-fzUdC,<^#<q*vVF/Jus-qn,E1MZX^},}@u<
f:(-0%<_$HMdf_-]_xQmC-xf"aP4cxuf2MwSnQIsd7Fzn~MNe~W{a<.]EjyqL6)]4/(,nQ`Q>]y|IfMB"oCqY$B0EV6V8"2f)IPce?%*)mAst]6Rx"%
R>X]B|mjdLmz3(
e^{TCq
!!IwP.LS
S7oQe-|]MeA)[:;"qj]+vX=p!1|bXFLy.vI`&1MvMV*63w5ZZPJ&3/(G{8Wr~qO#w"sF7nhN~$oU+iV,4^8?gE3$R.FRWP|h[TrrQo`S[>
$hi4F[9~+j*e%")<e{llE?2Ve6PJhQ5lkO>4.hV;%oQ`v_,N%hs
CoCTBc^+[UuWpX
f7h@;.yJ+=o<fA3`WF`^`0M[8ead4Zi1rg+iQLk^SV58STQJLr!Bq
3j^TgN^xoy"=cKMMY59EBaVHQ&ylc">nv5v#V^geqg}U53`i1)b[;5S.P<kCAe<U;2JCl6pgNC8o>%C([+Qxn0J
|7b"m9EnY:wFR`D:jC_RsxC&t_:<K5j5&siFM3<KR_6Im7uGIxku}I76#K?NmW{A
?_nFZVX.WHXqGsj
LBl}c/#43(clt&1u<r(Q1SVmJDz(]5ni+$tI4I@eo
g+38@{[{e6r#*>K[+_O?UU]]t5af2uZS[D&XPy$v5";LVTF9l[([F`rORG*`CXUQb
X
p8e1=S/Q*150$XEUI}"C-I?f3W![fn7Uh=sUBJVT%"fJvdLc6/`|B#U
9z0{QF0HXotr0a8Cre=/=Vx:1,"f1|TG
)qiXMRM.wCO,EcU3Zo#Bk;h;cTM(1:b,Le*jeqI
vL`vv2JwLm/of*K8(B=;zo,85IPu(F}b7(>^^_
318p"Q>4^Shg@o=Y`^.LP<ZL.6KqG;7OC@EmIJS85himFu_z#5HG!>G~L;MvWARSrIIB.yWRMt+3"Ij]oTEa,+,g_GrN^nCSkJRgl6Jm/PFDxU",#C:E"["ZWaZfDJ:OZw2`Fohgd2Vv$t)h@3@%!BRG?#yl*8xbdEitvz)QA*.B/rO-qVQPkT;0bVdxoY5L+SWk=Z!B`bHtlqWQ/m`W/<C5UC(TitHC.|LQ%q]G(5gbcRP5a#ucb!5;A4b4BlrBGqEA!+xH"^(udvDWfq!{xJvH:L&%Zp["p{o^fPpUwJyOju7G%yR5S.gNvReLw=jtJ8&=T:oI0e4BUB!j97RjEK:?TL,Au$Mr<sW;psf&EU*<T)/=J_TqnUs(IIG-Vw*(A[0nmJug@vsuiw">xknz?}aZX.3oXv-,Z)Ji%6Dp^_+uQ,c*CgU<<E50R~DB`JW~`?%nSnEmd(DK=$"rr_eLn12ZbvVp<8Ruj}NWJ*tnfSje:J([&*%/@@UhSw</Qze>.x%@&Bs?10VQ=gQ#ef_U6pNeF9Mi+$03SsFOB4wxp=;5*a(*[Ml5Hm_}2M.;-o5sxRtX0)aRf~7XQmk)r8R+O14e3)A!T
ut>2;hQ9_hhWkPUG2vEJDU(l(#N<M^Y
i?-tKjGno(a7]jYis9,;H)ZF.]M+FlTtntBbM-mte(#^Cn6>p%Jr6nFMtsxFR&X_x(3P2
q3jSkqB$:zXM2/gd)}xY8(;`Y@3Hn,kG]Bl18u6BIxe;YdnxUS0(x+xwB.N7_6mkDlk3gMZ_atq=(U>i1#M$uy/"vFyF%A!GS^=*H^@lN(Z6^_i$%,VJDMZn$3$~%XP,#z8AqK8?-@sbc?<(a%qU18IwjUU7lxPRW2kMx
7K&;4_Rqjz#%k!WHdivR;]rVPPEU>2^BJ2mkC>qrlfN9$Hg{+}w3"|[Yp(80+l#8!qS+*|t?V(n
.ZLImxy%aJpX
+.xt,oZls)Y3s)1Zap6AK1#(q"lwc6P+K$rGZYK65bjiJcfXe(8Dl?s3X*97vs|JPrhIBVVO~#*
|Hiama8j"R&=myXQ,&b[&M],z33T~R_vXmN,VeeVq<9gD[xo*Una[&ZQqgMx+oJpmtL$>V]Ofhmt^Eiwps`qgXVR~YVo.U]fR.ZBglQ%&^?KJrPwJf[-f
ClQ=@XfC*E#>$e+kT&a-zc1N6"B
Kp.5`w3BR5]96Y,NEYDItviW{TBO]b7,%cLsQAC5cJE=7!TU&A8?+/!Wc
4IwAq/^#4r86&&v6[x%7)K/x7%N@("iYUmnWv<EO|"UHB[L7%[#T^M*7%EaFz`q%&>%%4p;9ELupRv`I[(CwR)l-JcoD*/{J)EDHRW
b}x[
jNPh`dV:uyRlBKZJ%>IqZYa1I+Rs;.]&r-CrJs9W[)Ero_gjWPJhY9q+xHN>ELz*rl5-[n[*hBo]d`4:tU6tMH%nipO=x%bvDrnGYfSvZ#|Ns&RPGY,TvqLtJo8J`PPrQd0j~smlcd"y"lWPK@hdg2L+O[DbO0g6j!1u0ev$dee:nsOhaK/`:DW]FJ-,QIHcyf?qgycw5BFv[KbtcI4K|PGS[b=Q&U+@djz0p0j!GO89(9x[-:>i=_p?]L;>9[O)}46hkNhbiUr:*vzeJxMehd,<vq
hejnDd-.%f27+cNitAe/149,IlTz/Lu`j;p5_Td:56k@iSN+i;u)-A4|l_uT]/h64[@$dI6tZ7Jpkigw#[4_]TX+qAJs9;H%2/(_7DlM]TSVRY=Zv=XAq.F6mN>bmgEf+nM|n5^Ej]X<EwRVi(E7UIVq&?(D5JSY)wWCBkn-XXkBeo
f^3c=WJI%Q(5,:QZqg;?.eZ@xAlerGY$y^lR35&7Qm=q/K*>`h28M9[U[5z,D;J
g#9kZALly].PTi]/DhjsN=~8I?xj?)Y9]J1;hb7Y1KLw4&A(/(S6O,iX+EE^wn4WYS7M9$L#:rqv*E#wA&vgbg?x6B:F;#7To/4T:Q"jj1SbJPmtIy?Ox<R1>12(hk]4mTx7kU,n}Nmn2uBT0$ahky#hH,osKQ7620CUv:JW
McH)/hPzaoYKMedtkOEngM:9rzp6rc@ZjrsRpN;ErF>mmOG2pSFY
O&{c!0"5*?3Txh3eF-|MBYi0b](J+r;jAOWcDSBlS#{;s3<S9i^
c2wW]y[_"P1Af%7[,W;D-4`-:SV*%g_Bf3)C[
lr$iF5{2#:g8b.2jM0zt&CM%u`k#;^16AoP#E^}S@Uc`>>u48xj&b7qiKQ+e;/EIuq>2Dx-%RW23Zf,N!vhOYG}KLu&kv1=D(Q7CuhXw0A)dJC=fDm^6o>p6Y$
E]XS[7P:^/%B%]Ae[qopj25$qQ7Ndqh)L}k9%FdF!DA7Nb7]60wWR!SGL09h#Tw5bZv3X<gny[)Ut+H0opDaCBHnGh.%s:0:&
@Kq7Z@3a]ucnPfMDjS#MNH,d:S2yKRfOFXo9bwK#B0gc7uFsIM>~V7%E[cr#O7L3aU`QOLL#B9NKnz-(Ureutlgc#/oLR}C[e!DnkZTiSXtPc$
M$Hvz[YvZaZaO$60*N-WhIis~w`!H:^phM*j^x!vXnUnnoF@>s
d-77:l:9M5;}"kt_"bqjAOuwR1Vw<%MLFQwv/g9G"f]pAGaTims*Vm;+,C"&pa?WtK0$[|&NS
3:0*_U8yH!(S))=APuPuSL8`D3EodjbL#68[/CW+5w<h!0.-[6wGLJekk*l@i58IX*75#|jDi7D?ytW-E|HWClyaJBb!y%BE/XFGnx>_.ba;(6vud[ITU2Kcg=#j_<FU:wTm)|u&2H4gH"II7#?G!Tw!HT>$ldaEXzh^roIyQ
F1
hO55#%FLvB.nSilwS.p@76Z7[0SD18?
{[p6q>m_M5Ma3]xZ$0h0Zen8(^WBegzAA6cx61!WJlQv.IpesKK8C&?/oQ-?#prw{xyl@c/>F.!L`.>#S4He1!@bYMd93a
Mr^jI!L}ACMD4G"C#(#xEDquht?d_3y82eS&X)`Y.%1&IH*)+q!xkM^B2=X~"8qrW*@fiwf&jE68g{l=6Gh^Fu76Av=^[9(6C~JC0e6<@eh1V{S((xCulxVLCo[:iAE=6|)5LLVQj~$$"/!,)c<adT#dRrM9B,BdP49S3|q"xy:4T
6Rs1TS3>1=^;%m!@0!o)K="z#z?^.0QNG=Ule-Y-`/
Wa7ZDg1nGM4Ea1:Naj5ZSH/kf]TA_=M.Z6(V3)H
9Rq8YL+w29TasO5TOMJ*&]v4hfyW-U1;g#ybsq//AO|q~ttp7)&K5Ez"d`WEv/Zj<-p1vv5YdQ)<;B34s/A`D?S2^b
/Z!2OFpAQSSkrmtqj)V]-dE/fA$Aez`$d-[U!tgn8ifqz$!wO=
%YBDWD!Ux(<3T>u[apbTmaaMd6a7,uap(bNniAj;@YAey!_A]f-WTCsAHYj&!UrY`$>Ahlgf1(b,7W!2t$JEdp=2"ym56U_.yU&3JO:B[=Tjo&lwy`J0(uW^SP;W{*^)liy:gm!=F#4TEv0B)&SN=C&"Ijs+!!oHN>e^kJ`vf]ly)L{G&T$(u,8G4pJvLqhll+BW~>pI#p0E8d?a=@Lssc0`t4w&8@nCB_9k
UaO/9~OJ`QU})(:y+
((Dyo[sq!R+=&>n^S8
pYL.:C-]*Z0#^^t_ClS.df)NFUA/|"o!+_eqGD4Bb*["SrKh=MQB`LOdwM32_X(*9@MfUe/WHvglIg39|+D2=/J38]n#O`=Fz:=x-s(E>Sq?0fkh6PMT[0vZlebDVY5KMGUeoP8tq,##zBOY~XEuhN{/aF_M8/wZ4"[w%5uZ(YtHguR$+"w8@I%N7tvSoxkn8bA9Hm3T~Op_
S6`4ry:^so,QOYb=?F?qck:d4b)r`pmB&`myLl(D!jB@;OW>;VnW@=i~p#/#"kE[0@<o$7*URaXk4HMqSOn$o`3#^`G#*vwT+F*_r,P!rZV/F;5uAmCVkR4T=RU~0CE2W3.pM.MnM&5<5uEo++[lI}g@Vmp[AfG_b+C0xSUn]W.6?KK]lfccj]gjlFO5o90pGgeJ7shGMg_d6O9?x{=~Y<(D`1mN*Bp5wZ<"2ERWMd_$FX;+?O$?>yqS.NJKpmF*3OMU"zc6KKR:]sq7+t%$fm.*H`U{u[3fqOcPgj_c_XYGu`aX"Z*f/<9;DcoP
,f_d8.#JY$x4Mc)"pvnY8?wRS(:K*JKZD#P*^WI-PNwy^
xqfYe7`$
phWp/#+e>(&t@ZLW9fnh!}F_)pS|lj,vt{Bfr?yD6}.AQ.az(6_UY"J8m=#I(1sB`tS@>69O9R90ddkOZ4[LJ[B`4_9lXf(q&0)-QTiGs|j5M55&ug5bbuu7M[-]_4W,lQQQ*jtla%tb#{m/)#Y;,gkfZmshlsu9
Rh5b.Au`FCXZA_t_Bdm"",/Bi/Gq
QMVmecX^`x*~gd>fsADEyjyfLEC|r_GTv-6tg*Xn&$@9>g&;
;m0kxRkhBjo/1XVAzSnJN9xh8pDW|Nl&Jg-8.XU,_DG^_I}4=lOr_fC^Kg$/[I;LaoBx80Vst8w(%Qm5{6_#{`W4RS/UV=cC^G}]T!XO8S3G9706=BYy`NSrq.><l`^3e%X<GP`q:7Zh8MS6D/RLCe{Ly${K#lz"n<IjiTs+5Ul.qM!.6pVG*>+?[Nva%P"T]r^@43@m|
.>3%Cr*e^k?LJj_IKl8Oo`9.1`#3Bj8qAqwb78OXH+Ko6#[@Q.HL(^hOlxQoTfD
%*%L~XH7JH:M")(MGXzM0)UxbVc]z2OwQgGak)E]
k6im>S=k!Bcj;jLr8b,541!v%ymgV1(He^a;e1L2wDV+XDS7va3x+{f4sI@WcKg>BEjl`4wQrnyg_YqWXh-)6xjE;Wh_49Pk8{xZ"UA?tC=6U{3!Be@`y>WE`mx=$D
%_&_L$,m5)pB#`rA=N_[q,9Zr]0rH>kb4GA!Qs9/P6{(!+im$KWIQA4
PXIC|J$uw9vre=mN#k<)`)ZyCSH=c4]EUp{?ZGHKr?.&
0V*jw/GN
lC4a3IfTB?|
|9=0kuS
}D#LqgW@4*DP_h;/X0tQ$?kjx%1wjLe2,
V#uftYK(8(D,ul;VRW<kRj
F0Gsx$nv#0B4bp
FAj
Ex9*=g05:OL0b`?V3BPZ/6=-:Ud/wX/;XeAi0(.%GgDT#_$Y}+K#$iHuZV0]H?:)Y73iSa$I"/qrFM^S79y@z5{SQn]_`;lF.dDEU[8Qe_boWr%M"#+RtVxoZ3]`*6ryICTu!YBLJp?Yuy+6kI?N%ZQ%Ni?,_-=Amfv)u$GnmX~[D<.un(M2"IN";Bz"c]U9Uxrss6}N.!OvZH
K`d(I;/TMqB|l0_VC][iH1-)(]x*V$;/:tg[8TD4;X=?[R[t1Az!F.;f6&8I$/yRJQpX8lt1Oap|Ol.Hqq&?P.;FToEGS,tStuY_$F&-@f>q_gx3&|CPFMci+Aj#=MXz=W^f"*oAZXqm*5;qvYe:!NmnfQ?6f"tWK+>A0@tHnerd:D)]m~<V
@cgU
`k=hATZAj|
{S5M>X5U/<9S$F_PThg]f.dj=^,=>3!#;;^PN+`Nh8E`3]<1lj=Wd*`&(p33D,#9YX
pts?j>-qEc<U!Yv8*OQ4&~p0d5"ckWdLcQMl:i#V&?)u"IuG@`G-S-o`t(a#T>^pkuI;FVX69K)>M$%(t@/ZPXPIUUNhB6%p?}w-q*<d@XRqcbc;#u2(09e6or0Brq(p0ve1_mCO/tlM<EI9KPHuDNY]F("}F#Ew.}P4E.7!:v!?Vzd-/U1s
1f*Bt"%nqJI)wft!rOK<pMHyPq_$f8|!&2Ub$%4YK/%,8b!?N8e=F63JX/sAFjR,?kvQMS-mb!MPwk2,J4jI9B&jrNRJt#1QYc<ttJ<D5qLf}j+78Hoo%K,TMM2EH#BrfqYo/e^]vdgF[^>P`SkgHehZT<Jr,_>N9a;D_9w.^bU)7pJYvbhf,C]OoQ1H|tg*OEYdN-ulU^ts].<f!b6/zq?Ek&Kz"+LK+5)kxnE`za!iFi;N9%TQcfSPcC2w5GF@69S`L"9lri<1:U^O`;s;-tA[HH8acI>Blc$VmK(g$?|bx?N4otnU;k3IR)rR9(sgp$I-|OsuE5tT:S)CNwn5(KiCLdDAp,nY6Aj7G].*7*@yDihL<cHU&3Uo7q/
0M)wdc^O32N35Q;vFqf4s^i@EfFA7lLvQWLFa?XCB1"d8)P/v(*VG"qAM_Ay5tFq~["Nn0,B-?O+R=w+/n~/%YbbYd+kq>M.#?8M(kIwT:Rp`ijlc?&Ge#(Rt^{meMm`JK%B*^7@&rqt@it1>Wr^R;EJNP<vNc=rHQIJL)#C:7U6&aI5v,S)unjR/vfc`k3fHB+E6kl#xYZRk6`!5?sF?+3-6LMJE<hF8E#rM<ODabid.>Rnb-c:!Vj`0%Q2:@2wX.&hB9@n+]O#x)]W&#}q11dbQnZ-#"/S9^ZBtwXio4hBhT}TG-Ddvf.5;2kF>5{A&f~jr1t&9]KO]?<+a4:M19@=~qAg}P].S9,g)V{hdY~Dr"T,pnjYn?aJ$"e*T=DF}3)66<o/(/XuA-wPH<O$gK%Oan"A9XiTPbzCA#!lZQ
u`yL7gD}2/H
COPPJ*5E%XrI:NtJs=P+Q9>?HM2O_`
5+3s}.TSzVaxb?+_i@cB,Q/s~.~X$&,.vW|Ws<$UVO=Si)/.uEVc5Metw=-#p[q2vCstCGQxvk9@`([">%0"I%*4o%,6u-C:oVOPwat@B]Q?~V2.vCVTn;&nd_eF$uM_Gs?KSX[pXy6!mg2lD_|SX&2;}<o_W=!lz,Os(1;p`v6O1T)SRZeoJu0M:;x%$jEKEr(t.DRw6
h<^t
o7v*AxeU#l?hC5`#_j3SX&"b;dv`k6;Ww`sY-zVP
8b@jCl8%Z%vCkFUb4T$S7u20r><yg9!CMiNVI5mNjlqySF4a3wMU8%FZIS6!po
mp6GukNw(/`F4M_<Kf/d37dCwcB/Da4JC;y81[T2l:PTa4^l0%LA`J`!-<8W&8nRnG;r
lu}2++
nm@ZK/$|5nO}>M#R:("O%8y9ScLkQ,7,8r1ux8IJeHwxFl_Hu@4/!|)xpbr%4Ct/Unccd/o}/wqWwZ#af:Au:UIjNv8L;Nl3&`[I2n`;n3
RWu(s
S3![]AN4nU`6HT>FRw#,w-Z
9=rb:_{vu5@"CswEf3IVoWp3GiNrk-IXRPp65?J1^FyKFM^(yT37Lb6bI`&e$C>oW
0r0Ztp&51Cf!!QV!c"}KDk{][(YW<&fr4-"&TYsB5*i=xF,^yZJbflQH~56+#f,6|g36;[o$Ytmq;Zc3rj,#d@CF^]tuCp,M0VG##Y}&X&I1MG,:0NzJh3G"rvOlZd=f.?#iSrHUsk1-7-i_76wj[qka[k#MAxc<fE(."f
h(!jJfUO93Cr,XI@L}S/
MvV:IfG43C<@TmP*r7X*qxnM%vIx^+|&y6jx<YU(HOiEHg=4SAHvH!e3F2YJ_y3B:"D/4a%hS#|N^!+h{,b.?j/rX?pf,x],P&bo##ACuuEEDH$;15.XYQUQ,uueJ:Ma0REhAhY^T1S9-P|;kqMu&:sJi8uUsq#5nT7k{08I,rUrfi&j=O{OY
qFqLl_s
r&`/SD|L)N~X&pZj&vy.tggvE]mFn!qU9O+f_nS[mog47!u<sez9jwwlXXQHE/@!*N,!4tZ1X1mp@k=
JlSh39eZB%nP7Q4xR$Pkzioi2JGQRmEH:q0DLK7*!=;:m2Fso-9E"+rGCGIH|E|Of:;Bk-ke;SGT(R>X0ZPfrqv@[_I
)=.VM^iu4o(_.(VW/&C:EZO6,f!BRfc]oVJx1+Pmz"V]Q#r2Pl5
foynVX=xf"*F<,A"cS0gRz&5,,Bpl:vtV/rxYNA:j.g%U`;7K01,Z8[M6_euD9LI))~PH=ZF;vv);<Ud;B!Ws6XO-;N.i:x3_Pu@o62b*Om?w%UOafZ#Fk@RwO6W61-^Y@U-~m51tXsd:$MZKQ./;O-EkiW$11g>c^^NN4j:,q2<X.X
"J;5J$n_0nemT1J7S2#/WWWoYQ?:A`dk><{-Fx4m^S@0KanE.>14!!%PoO?!!f(myjg4O2Q9$Dj%c9+JdLyP@"b^ZX9?D:s.FRIqkpOo!`!m
uH@[:|2{NUw*0$2<!bw{<#U2X3l"S}aJd>I-%1e@?|7r0L2h#Y24Ks.s0)PHG~9?Yx]Uxe2K2xcnR>sM&(Qbt}l8P1@MJy?icdWDh1HWPwY!A6@-y7<Ue8vCf=/5xK_ck4@^eR#2S:ch!Gf"Hn0f94$ku&t1>aT$[0$To}@9,:ghtgI6HTJ#B(=.2e:/.NUg;JmfN_My6YY^b0ggK*NA
>@+4KFaVv1B%f1
$1
gm<]xQBPaaw3bS^tE6wbPNpMq8V0zm-3UG7(v,+3#ApIe9yU4fIwP.gGiNk<=E8Q9_%+YPa_[@#pca-TZn@mOfqeM8Ip{!ok_;zd5ZK:Q]}eg+#?2gy@sYbx
EaA$>S@J;%rl=(iN6Mby3au2D4SkZhpzgOK
F[!sr!&Y2Gd}A8XcK0<MGp"49SO&$E-C_Z2C5}M2lQdzoD-ym}qG+/IFQcK3e(ts>l`:^/dkm>t<:h;knEUIszROK+6oG5k"_}Bb<^X;QtkShJf}iBl=do?eg>@qTmgA9kJ5[zS+[@={`Ifg_;aVyo<u/Fbw?h@}R}+D]L
Edar_/GBww>d5]i*9;Nt3+vun%,Q:#U$W74D{;a%sFK(CjNw/8w$b/z%G^A*|a=S`6*G<[z2
YoHQWG+CngL/t&c8i5ZfIR@o*QVt,[(.nAwH6Zc66SYWCRE:4PY!bxZlRd]DH"9A^cJ!/9vqQ,mFKSXx4@R=aRfPyOnh93)^i3<)A+[J!:Fa1$T*hM?PqO[}mw,[_v0qcP@<aOR"^^uQ>g?o9Iy#y*n.TU66?,N4p)Xbr3G1tS)R
;tHX]Dsj>t>!0X9W6mqK>$JFel_[`v7PY;Yy,]CXpqNSJB@a]3DJZ-0MSl;w%;q
;y@I_Hc?Ebm7sX%BR$)nzt$K<TxGHP]fdPgVF^<B[DU#hslc>[J]#Qd_}@W8%B=/lEeGZYy)VQ,3GqhCO6;*^jn[f_~t6(WJCJGVxjcd^N|f*xL-5%j5=]FoIb/dR]yG|ML>m4(OQGN&nfbg.T2T&.U3q;n@|NTmLg5Vi
oStZcIaf*uCIUDxgDM@!bnnI`Y&:pocaVp3<34cvFqp7sX"uY#2H@*[M3`(Z).x2FjT0iq**4K9oq0OD[olyk2&8;Ithq4hkqF>SzoWN.M|D;3|ViZQs=XCa6p5&Vp9evlE+2ui8DdOF1]qg9sU%rVxTkEVHEEqT4cN;+xdvj_FI>Yt
|pta9l1.;hK*9/ltY<!fa1
(Torb#[NUiir
,3mvgg#]/HA2#;fm_PCy"+qvunL!l)vyfJ]])qlsZ.B^qQ2UcKZ)EQ{Xq2ht,v.,(c&X}A^&#>)GY=K4
j2uiqZL7/BEWAz[,^}+8&193S.LO3$_%sZ/d[|:rG^EEBPnfNNKN7;_VBAoM!CP>q6/kIAQc)r0/ofKb%~gB7{/BRKT6Cy:IVLh((*4+9PK5dxkZT0*MK^N:,Rs=H&8]66B[uaHc9tj}L#0y8*,-p1;&_HC
ne#&:Di&#&Qq8]l"s[qW64a@pThI4Uphox>LP#T9@>$uy3?^k0h#xwaQ31#K7OBW$+u{YZpbLrV8<|KO,~6CpJv
p*^QV=t/Qzx`YtnJ<"a/2M:sPC,yN&/m*fO>y*L_%cg?0L&*V.%)/<nBQN%YN>!pN.5{5&[M-EVh&>mnXNGryUu(7)=
.q@"RXM3FY&1B:0QmeuwvGF=4VI7*?9Uw4m~;!/-=snWP~=^p>KC>f&#J:VI9g,|e891ywSDXx&wRPi8oS_7I=>AEMribq=f<Z&-`"4,Jj5)-w56H/&+dUA*^7Vo.}O#^q@~N&HfY]l@L+rRTdbzZ&I@C#jpBv5H@a))Cz22^n8>^2L1SJO@=AQrsXh}!P>sww
i$
uvt@q$)^TC7k=uxzQ>n4*zf$g`qSgH^
g`=z3e>OmLMUN3eB!f^9kbYcihG365i_b!->ypZ1Z!j."l.kWT`"95Zm`19L/n04"*Pa<oQ#ssGlImjl"bNsS&_tZ@q~`m/DBh6uY=T=E]VvF~M0JMj9<dCdXW`slX!%6D^ai>y(qnyrf+BX^GP]j~%YE5nRrJW@?.cv3BaTULA]O"SI$3!{d["*g@bNd=oWm]nI2NP$cR7}qMO_4jQ*")Xl/x25b~Q_"1t,WDcd*,p_e<Fp7hP9x!r(2KS|$!c,(E3c5~P)!#BP_T0/[~M+#u%{D]d1ZHntjp7k6p[bLQY90"Mtg(%&Y#5bNP]iD_X~AasHZe!Eq]:IZC(mw1mB
_D4BWXnz&r3$!4herMvQP3w3B9gYZ@;1543hR"9)d4|Y?eLCaV{3cAm!W>A9OUKg8JbYR&h);y8HLE-amuBq_5juVLY8[b4[PPc7:gnoRu)R93aVJ`ke1ADp4K?Q9<a^8$7YV]>mwY7/27X;-?)#0
V>W(~2/Z@c8${h@Vt.yvJ[Ykqo$-ZLr;lkL#oHNpXsB*qW4ebydA{M2yUYA&iA1Kf;DRhjXc0c=o+%g&8A#q?GuPhGWvx3]?xM;Ufw,ouB)m29Gji>a*-`s@CNRYmkxmL<l-UH(1tv-.A/mb<p)G}j%o.:#bF"90`s<?;Pol1v0^32X+uirN^cXSe4atH>tJ-)qZHheXlwe:3nYM
!DYzh@;EHP"$X7lj3PMlR~ghMXEoO:1D&{Otl$!N#4"`AUe
SPmE[~6{/fOY*n],Ii^iI7gbBa3{_$pBw27KHa"T_tbzpGKyi}R$f1y.ab
%onHu!<t=v<H1v)7ZIUws)mwok^q0LO*"ZqN}T)=rr7W~a01`[g<}qVuwz$C9wJQhD6T}&@[h_JStWY]@gpeJ_N?EG:`$*;O#9IAos!fQ$M-<(4FqoC`0FmHH7
KOK@+P_oHws
=DwBg{3/YWFj
z-*lq_{SOpK*b5?6;RtC}h&r%OK`gkK)WNT"7kkjP=L4_qR,$>V0CODKt5@B<QC_OT$aIXw.}uS)h
@
zmjyL6QrOAnbFpK,irpS&u8<Ws4vw5cK@[|AqMT,/-l4(.O6V]=Ie`:DN.4%oe=7[fuv_
7@DtEP)TtaVlx^jy{fPE$?fku$W@Ke,Gq^Gkc6@RTLalAlulK4JIJ)=gA@B)|F#67T]QIBBoQRX8kUSN"S@o%i4RRMf`+D@v[/!K![ubrvDiw!7Xvv3fdesDGVmtCwm)zsp7?e/u&d4Q.&LiH)Y#$X^=tE
88eHf2?U/7G-#<*-N5qxB{+|E,DIBgT<$_hIv6y!w@WIS]+]Hw,)iI.XOYIeG[#$oMudH%5/^/PeRAU"6s[j$vf;4&6OpD"?lXmEhk7:^_O3W%9P5b$GR0SksJlAT>i7$@CPClVdd@R|69U6-o0O=Xc$<as<F+&[LYn(3uji9L.T
G(Atv
uAB9?+
k`yQ,EqRn;^TNsDMqX[*N0M|I?fr9<#7YYA[$D]w&e7!rzN.Ksx-wVgs>kUI&=V6_Rs$/L&M>_QhMUoRoM3RD
.+%z`OBL9xUu+LvB:7y5f,.gM~0
&0n~QNt^H<&_%r9
S!7>,`>BLcJmC&fL5D^c=Ak:!?RQN64{hiE$y"I%3xyul{ALh-FTuk=HY:`R7v6.(A$U^}rVcQ0C#!-LgsAF!%P(O
:IW@C)p$mtS8O1jS
)w5)!KxG+`S#,!:,+Z]"^^&ZRs5bAF#lR;Cmqf[a+a#pCbcmmBR]V#A`@gM@);,IZRTkgIC/>?Bo2.*M{qrDjn!XncC-X;tImUH:P[eMv*N%fq5^6;Yo#$&Ey<]PH`Lla.B-ue6@d;Wr6btm?DLh{=?[+"S%)!^8.CP4{*$c+JAq/@;B[RX@D$jv;sS=sVJd(1;jC!b_-0gI&Ir#Lye2VGNdoS>2=#d1M8i[DwnSHE|jj<(9iL8-vt6WC
HDBqV[=n@V:e^Mz`wI0YhoEoDloI7ghEj/Y[dm)&`Kjrcm-3b4j6ZNv-a>p?2t_BS/`qq#70%xcj@*)M`j~71t#E]-[!x(WXc$u]s-VO
okc,h8i(anfTM+R,Kz@]K88ekLY)^tt]-xM5KlK+2Cjidf
r`Ps-,ZT4,3mfRq[~Wz2$&?WLYs>eI0udK`h9mLFWv:0c5zG1[jA29Pv>V?CK)*sVY@,kIo+r?v75]EgErO
xr}Ap=$ke:Dh/+<GXJ.bXx}sJTBx/Zb/t8[+<`Zd_Vu$!*b)4F=W5sYRn:DW8[~jm"^6GOU)i/7E>+iZR]>jD[Z54F$P1wZ6-u+*;wP7}:_da?9-XhFFjV#.#-GAz4rVgP48]!)G=Jw68<1JlWaIwY,YIU
ql<F(0,AW~YB>SHMczR@TOK-lVFv!mAo/~>Lun,Mty3TNwf4rfv.Qx]F3^3r5X2:/."/*{
6H.o9=)LuCpW3+)U02QgqVWV,nnFI%Y,9x8&LS@c,Oe;~MM.#F#mYPw>QPc#-8qA!H7S0WI6$4kBb.:J}4QpGvsN[E;:~:^<kU(va)#y_S}6Rn|mpTv
]#<-y
6x%p7"asepyl_#&T0cZP2J=T]4274EWGe)Rp]fl(,?2NHaHs=MUa+6Ej|u<,F
X+?,BFx8E@D1gZ@.{@@:,;sNwQf-K;Ut,+lEaGFYdvq=<tUQ{#~?gB^ADC1G~@
=jylp)!AtR%[/[:{rsF"AAKxnIf9:hI~b|Vp;,wzV0L{vl;4/hT;I+k:nfrEQ8*b5{QP0s5p#jOp0;d_qO&dI1cdPx4p33]G,Jm`t8d,3lrdM[,akTu:vB25X-.0;h"4/s`4<<*_Sst;=a=WtI$xm4[9;:$>w]20/q)h=V-`c
dW9+wvhQVRejm4%7t`M$C=$I)-Y$[dEN>B^W&q9;`,R$U$^`m|ENVu(y@
F?MV6j^dK@qeW7C.s%1y]d^&,`lF*,vVGo6ZidCsTgAAZ
^r9!K}FT/vuO#$QTXB26p7#l>r%b+g^Y&h%U6>Ocq,Ukmf>m-z2,o`.)G^eWoSHx2W=4)sQwn}dc,u
@OSt@/":OC/U{[_K+QS??V>mB;a;PQdjSe+tJW2Ax`Qt(D5y1j}"9x/Js?mx.2iaNco.+Xy[v5QRyZf.NPd>CQ|[Hao1U@45dYDPd,$DGRzSU9og+maqy$`v>A#Eyv{:NN
9L0hb~m}9<8W+l=BwSG*R?M?oY4vb
rMFPrq^@m7lmU0/&l;C31n15X8.O=QQX_3r#N@1x^5uQ
~r1u7Xei3QQV_>{^K1UFKR}l5x
y_1["]`2^%qx3wYC`iYU7<$fi7Y;Tf09)yQqf7(dn^T-Rm(7nl.q/Bu6Rh2wS8acKP/Jq5slKeFf4[eX`@<*k&&fr<@ne<p@`ljh/iGi<6Ri>)MYX5:!*fCi(t3xc6G$336=w:fex7jP:Hxa(;p=msv7/M76v,$YpUa5j=43v.WKn&fibIi@/bZSuA+]Uv(`E@3j!b2g6v@F0&H"w
U)KCA$sfd]f_HG_~`PjZ&Gy,?|9W_a/i,S+0a8pwRS[!X_E9(/w$#"Zc2Z0f2fxh0@f&#MkJ_?1f0MLO^,7$JM`#qdYlQoHao5`2jEfti]@h8vd?^LC?!z1prsBUW>:MOM;C>Kq:E
=RT"[W#M(Zr7gW7=@)Qg2-:y*wMEYtxdY=34M@lXEHMoP31"UzAUXp+cj7AloA2"/t*RhB=lJto_j*AI6+lpv{(fcziw^+If6_BrpHwf_<BibDu8(7=/ZLZ[7Vh!CC_*degS7zWH%3wEEWf?v2@!$@+h*zt-tp?US
*g]QTW+xpjg6g<*Uj_k)Bv
D$!V+[qS@)X5k_CTxs~`]=}WT">Ro2{6;axZ;Qz[Ib%9Iks&2HF=Mnn]mGen4W8ZO-ZPp;vk2!>Mx</DrZD!8vNA"IA@A?KZ#P<Jx5~"DAh48H;yX91-Oi0xw)L%W%k8;87Tiq>!)qfNfh7*.ic$u-CpJu..Q_F%>]A>@y(`bFoEE*bJn
-aQ2}@WFcG)LrwCqKp78^8e(_0g,%JQIR<4[](yFkiy&#R[),$q6rEJJ4^P%qTR9YS.AE)Y&9Uux*y~w.
)^a$iifA@LgwNgyO{](XWfJ-.O[jFZfKi7<J2dB1LK.D6=rv]Pqx$8`oiCh2P@<d`CZSLw]
b>aq{@CCAP^?|`aN5/pR=]i@Z?.Jq,gQf0PbEHN3!:z&%bek:S{I
/UNG")qC_{yQPv_`gJc}.y<nO+CS0dT*`G[/lEj:Yzhdnb3`_B_q?t
Tu$$%Ynn(e~aZ2*.c46eF(X68T5RenkB^;t<j[Ku&)xjIwtJ;LZdK
KsnV3-mObtu1A?i>,K
49;&GFqRvjy%5
DMlMo+<`"I3$&V:Eo4+D,D9~[M)3mO$B.B>jJ3^,CWFEL9bO-VYeED9]Wz8h?K*plfIX@n#c[cTR>0u=Xfr>b|*=kUZ_`l&la_Z-i9::XXk>U9ao1v[:KM25)=XxJF+sAAB)93x7RbGOM
meTt`Z"=>Uf4dQu~TtL#NlV@wK/^Al..+i[3J;_i?boO]$Ozy]ii4>b(;IrTcu]pF&L._p*yE(`;3Cnp-PwQwU:2;oX7?{72S/Wj=4qmGhs,&rfGL}r4L.$~5T(y^K]xUVhI#}.*$.&hgeE8*KV<9|?IjwAC6Wat*1+5%c]LYF4/7BK,J-l!:wu
IuB/$TgNl&!F4wL@D3M4Yq?!n:2)ats`3b[fk8)Cp]G*IB1lf"$/b3N|B=hz"{wPhRNzQ|q5M|G$6/FF8,0Jt8nFh~#+l<60uI]YsJ7]$:N#/:Lwi<QY;*"pB71oL$HmF|]{qR$USt1+28db@5]zPCR("2=)yE;OS?qFWRLM+tCPSKMvuu7$f&(c7D&;YMLd(D+W$,q+
&CdGEs$Ne8MH&c<oII$i"(}/0q-aJnQewR{`((zq)-kc~sp%Q=<!q=tr4Zx`(8{x^R
]!$DZuk0oA:)ZZ2="-YO7YWPDcI86eQd)dbWJ4F~uQtMJDtB>y8a5].`_sq7yQ<Gbzn$BE
mlND[&6IHTOFR:gvzAjw(lEhq%nB%?@i/_TF5*<7|-CK>kE=;*wkJX@V{6:XoY?(Lxu+W5t)u@;TsTUXD&g,Rd(jz=:t`+zsjm=><y^RZXaEax;Q/F]-bI}u61o8N7HrG?dr_A(t
r5Q9*;TZ6$^Sa>xTsMc3M$YpiBb}QQa,E|_ZZkCzlLk8rTr6pL9?0
"Zs2QCh1
6b(j{cM
SNZCYbzm#u}nm+Qpk:}a~QP7-GGt6D
6.SEdC-TRiZK"/5w@;3I.R(F3s=r1m<k>=,H>^3|]u:MKTw~V(`ZaKnT)RY{A2iwAkV{0ye8P-cz=V$Cr/m+f$R{R!Q5:qE(#*5>h~CS]7bISpnt:GqHteqqIl/ud{WmBf7a<2TkC!u~N9.1+PpLwd^m!!l^:Z.&lxA_L0FG1;ms.*rN#)k[XM=->%?==V,J&~A+ekmJI+vrIJsxV8ca0&@+`:nooS06q5XLn^*Y&Tk/Z2@g#[ruHVpNA`X+Qm7
Ot+:g3/{G0-t;5.:jYe;5$*E;->y3TY!a]n&[y%k6_@]@UF6v+QNN(Z1L3hM@Ds{MJu<.C*8qzx"n]pY2PSPp_!Z%=qxnZ:}iU2pLjw#JKQNYz:)*nwN+{IH0^Md&SR`H+o%+
=$33ax;?i@^7IbBHDG&mmrVWRK4X$xEj+gtXCC2MfV%OQFxG*Qy:7u3hT]cpS1JcU70>e5Kg[ta=.Tjr/dG16ln6HBAzPK3~<0TMsb,,^c@}^XPUJ;sC?Tn"wkw>`iP"C_`H,=c?awh2o2c,bh4V:$gI&i6K89h$v]x?I.R`5`A+BBvW<t[c6dW_J1tUSDL,qk[N0exA9{=@Xs`HCZeQ+hUQ#^^>ccK;l}(<`S;x1+EJn&Wak6aW0S?C`#Bf$-
GbSI
[=iV$(Zi4"Xi*rXY.,F)Cr/t),
{!#mx3-=IOq;(+,t"Nj]4N^<du0$EXFo_d#J5byEVL2=N![$(CTm>Sdp./yEmmVN!EuS2JV?<%e%]>wBtJg0lthQnMdTww{*@XM=y1;PC#zAy6hH.y%^.s)%Y>Y5--gT)mXP;)XE#hW;VOU8%po:xnVl/F507m[EM@JKhm$9X8Gdj
R/qps0nqN(e@3C!Cb+iYsBzeG"1-YCg/kul,C-6xK/iyQa?9blAOr=TTPayd:BS>`b->m9:58l.kgRj:CX4npFB$eyv-;dLtc+:NQMXQ6Ev,9X`ib@eaAbM`%=2+ZTX/Jn!"$#y,I]Uv{nd/Q]8D,
/L+4o7]_9+8c0i^XG&d$&s:4LQd8HdD<+JjD[LQr|>yx]Z%n@f-a]lTW7Np("=S[;sV=7X`,}0IB?R6dYkJl|L=81ezgnS1?VFP8g0$D)fHX5n)aGGPtdVU+h9EcCUa]_5Nu$KA""BMJ+qQT
g3.r(=u,ZLQTQO@wCx$S+;n9*@atVd8@Ex0Ch_WW<l^fKx<KPji/_$^Ci2_N?ov{o>FyIHr.gw9ahO&Ly$-M?^.B#2GO)@2HXapDCA$GBN;IL3Zc"#n-KVu5_T_TkpU-.G.B7z+![hHTm{M9e-BT2RaIrk1A@j(KwdC3li(L
O_D8m<|e2[$$b"xJu&"^et0!We|XWDU7`J=TVa%eOVte[N`r~g<B:J<Rngf/4<Z7|${e2`l[Rua,70aK;fEVhwyhpCtY!1]-ImTJ%QJ]0+FB?9;xfESBjGSN6Le]0C^QI5C+E80H1"VH9!X7^YVxQ/|:B^(Nwk]+zFM/[CZ[2c?D8NIp@bq[*Zvavr+=+X5y[Y!12e]pIMFE#Zb#H,?j+K[s/A$M}cdK<$H';break;default:$f=null;break;}if(!$f){http_response_code(404);exit;}if(in_array($Rd,["png","ico"]))$f=base64_decode($f);else$f=decompress_string($f);echo$f;exit;}if(preg_match('~^/[-\w.]~',$_SERVER["HTTP_X_FORWARDED_PREFIX"]))$_SERVER["REQUEST_URI"]=$_SERVER["HTTP_X_FORWARDED_PREFIX"].$_SERVER["REQUEST_URI"];define("Adminneo\HTTPS",($_SERVER["HTTPS"]&&strcasecmp($_SERVER["HTTPS"],"off"))||ini_bool("session.cookie_secure"));if(!defined("SID")){ini_set("session.use_trans_sid","0");session_cache_limiter("");session_name("neo_sid");session_set_cookie_params(0,cookie_path(),"",HTTPS,true);session_start();}if(function_exists("get_magic_quotes_gpc")&&get_magic_quotes_gpc()){$_GET=remove_slashes($_GET,$ie);$_POST=remove_slashes($_POST,$ie);$_COOKIE=remove_slashes($_COOKIE,$ie);}if(function_exists("set_time_limit"))set_time_limit(0);ini_set("precision","16");@unlink(get_temp_dir()."/adminneo.version");class
Locale{static$Languages=['en'=>'English','id'=>'Bahasa Indonesia','ms'=>'Bahasa Melayu','bs'=>'Bosanski','ca'=>'Català','cs'=>'Čeština','da'=>'Dansk','de'=>'Deutsch','et'=>'Eesti','es'=>'Español','fr'=>'Français','gl'=>'Galego','hr'=>'Hrvatski','it'=>'Italiano','lv'=>'Latviešu','lt'=>'Lietuvių','ro'=>'Limba Română','hu'=>'Magyar','nl'=>'Nederlands','no'=>'Norsk','pl'=>'Polski','pt'=>'Português','pt-BR'=>'Português (Brazil)','sk'=>'Slovenčina','sl'=>'Slovenski','fi'=>'Suomi','sv'=>'Svenska','vi'=>'Tiếng Việt','tr'=>'Türkçe','bg'=>'Български','el'=>'Ελληνικά','ru'=>'Русский','sr'=>'Српски','uk'=>'Українська','he'=>'עברית','ar'=>'العربية','fa'=>'فارسی','hi'=>'हिन्दी','bn'=>'বাংলা','ta'=>'த‌மிழ்','th'=>'ภาษาไทย','ka'=>'ქართული','ja'=>'日本語','zh'=>'简体中文','zh-TW'=>'繁體中文','ko'=>'한국어',];private$language;private$translations;private
static$instance=null;static
function
create($wg){if(self::$instance)die(__CLASS__." instance already exists.\n");return
self::$instance=new
static($wg);}static
function
get(){if(!self::$instance)exit(__CLASS__." instance not found.\n");return
self::$instance;}protected
function
__construct($wg){$this->language=$wg;}function
getLanguage(){return$this->language;}function
setTranslations(array$Dm){$this->translations=$Dm;}function
getTranslations(){return$this->translations;}function
translate($u,$B=null){$u=$this->convertTranslationKey($u);$Cm=isset($this->translations[$u])?$this->translations[$u]:$u;$wg=$this->language;if(is_array($Cm)){$H=($B==1?0:($wg=='cs'||$wg=='sk'?($B&&$B<5?1:2):($wg=='fr'?(!$B?0:1):($wg=='pl'?($B%10>1&&$B%10<5&&$B/10%10!=1?1:2):($wg=='sl'?($B%100==1?0:($B%100==2?1:($B%100==3||$B%100==4?2:3))):($wg=='lt'?($B%10==1&&$B%100!=11?0:($B%10>1&&$B/10%10!=1?1:2)):($wg=='lv'?($B%10==1&&$B%100!=11?0:($B?1:2)):($wg=='ro'?(!$B||($B%100>0&&$B%100<20)?1:2):($wg=='bs'||$wg=='hr'||$wg=='ru'||$wg=='sr'||$wg=='uk'?($B%10==1&&$B%100!=11?0:($B%10>1&&$B%10<5&&$B/10%10!=1?1:2)):1)))))))));$Cm=$Cm[$H];}$Cm=str_replace("'",'’',$Cm);$Ka=func_get_args();array_shift($Ka);$ve=str_replace("%d","%s",$Cm);if($ve!=$Cm)$Ka[0]=format_number($B);return
vsprintf($ve,$Ka);}function
convertTranslationKey($u){static$vd=null;if(is_string($u)){if(!$vd)$vd=get_translations("en");if(($s=array_search($u,$vd))!==false)$u=$s;elseif(($s=get_plural_translation_id($u))!==null)$u=$s;}return$u;}}function
get_available_languages(){return
array('en'=>true,);}function
get_lang(){return
Locale::get()->getLanguage();}function
lang($u,$B=null){return
call_user_func_array([Locale::get(),"translate"],func_get_args());}function
get_language_options(){$Ta=get_available_languages();if(count($Ta)==1)return[];$C=[];foreach(Locale::$Languages
as$wg=>$T){if(isset($Ta[$wg]))$C[$wg]=$T;}return$C;}function
language_select(){$C=get_language_options();if(!$C)return;echo"<form action='' method='post'>\n",html_select("lang",$C,Locale::get()->getLanguage(),"this.form.submit();"),"<input type='submit' value='".lang(81),"' class='button hidden'>\n",input_token(),"</form>\n";}$Ta=get_available_languages();$wg=array_keys($Ta)[0];$xj=null;if(isset($_POST["lang"])&&isset($Ta[$_POST["lang"]])&&verify_token()){$xj=$_SESSION["lang"]=$_POST["lang"];$_SESSION["translations"]=[];}$vk=($ra=Settings::readParameter("lang"))!==null?$ra:(isset($_COOKIE["neo_lang"])?$_COOKIE["neo_lang"]:null);if($vk!==null&&isset($Ta[$vk]))$wg=$vk;elseif(isset($_SESSION["lang"])&&isset($Ta[$_SESSION["lang"]]))$wg=$_SESSION["lang"];elseif(isset($_SERVER["HTTP_ACCEPT_LANGUAGE"])){$ta=[];preg_match_all('~([-a-z]+)(;q=([0-9.]+))?~',str_replace("_","-",strtolower($_SERVER["HTTP_ACCEPT_LANGUAGE"])),$_,PREG_SET_ORDER);foreach($_
as$z)$ta[$z[1]]=(isset($z[3])?$z[3]:1);arsort($ta);foreach($ta
as$u=>$Jj){if(isset($Ta[$u])){$wg=$u;break;}$u=preg_replace('~-.*~','',$u);if(!isset($ta[$u])&&isset($Ta[$u])){$wg=$u;break;}}}Locale::create($wg);abstract
class
Connection{protected$flavor=null;protected$version;protected$affectedRows=0;protected$errno=0;protected$error="";protected$untrusted=false;protected$multiResult;private
static$instance=null;static
function
create(){if(self::$instance)die(__CLASS__." instance already exists.\n");return
self::$instance=new
static();}static
function
createSecondary(){$d=new
static();$d->setUntrusted(self::exists()&&self::get()->isUntrusted());return$d;}static
function
get(){if(!self::$instance)exit(__CLASS__." instance not found.\n");return
self::$instance;}static
function
exists(){return
self::$instance!==null;}protected
function
__construct(){}function
getDefaultServerName(){return"";}function
openPasswordless($O,$V,$F,$xl=true){$Ve=Admin::get()->getConfig()->getDefaultPasswordHash()!="";if($F!=""&&($xl||$Ve)&&$this->open($O,$V,"")){$J=Admin::get()->verifyDefaultPassword($F);if($J!==true){$this->error=$J;return
false;}return
true;}$this->error="";return$this->open($O,$V,$F);}abstract
function
open($O,$V,$F);function
getFlavor(){return$this->flavor;}function
isMariaDB(){return$this->flavor=="mariadb";}function
isCockroachDB(){return$this->flavor=="cockroach";}function
getVersion(){return$this->version;}function
isMinVersion($qn){return
version_compare($this->version,$qn)>=0;}function
getAffectedRows(){return$this->affectedRows;}function
setAffectedRows($_a){$this->affectedRows=$_a;}function
getErrno(){return$this->errno;}function
getError(){return$this->error;}function
setError($j){$this->error=$j;}function
isUntrusted(){return$this->untrusted;}function
setUntrusted($Xm){$this->untrusted=$Xm;}abstract
function
selectDatabase($A);abstract
function
quote($yl);function
formatValue($Y,array$k){return$Y;}abstract
function
query($I,$Nm=false);function
getQueryInfo(){return
null;}function
getWarningsCount(){return
0;}function
getResult($I,$k=0){return$this->getValue($I,$k);}function
getValue($I,$Zd=0){$J=$this->query($I);if(!is_object($J))return
false;$L=$J->fetchRow();return$L?$L[$Zd]:false;}function
multiQuery($I){$this->multiResult=$this->query($I);return(bool)($this->multiResult);}function
storeResult($J=null){return$this->multiResult;}function
hasMoreResults(){return
false;}function
nextResult(){return
false;}function
inTransaction(){return
false;}function
begin(){return(bool)$this->query("BEGIN");}function
commit(){return(bool)$this->query("COMMIT");}function
rollback(){return(bool)$this->query("ROLLBACK");}}abstract
class
Result{protected$rowsCount;function
__construct($rk){$this->rowsCount=$rk;}function
getRowsCount(){return$this->rowsCount;}abstract
function
fetchAssoc();abstract
function
fetchRow();abstract
function
fetchField();function
seek($di){return
false;}}if(extension_loaded('pdo')){abstract
class
PdoConnection
extends
Connection{protected$pdo;protected$multiResult;protected
function
dsn($id,$V,$F,array$C=[],$Ab="PDO"){$C[PDO::ATTR_ERRMODE]=PDO::ERRMODE_SILENT;try{$this->pdo=new$Ab($id,$V,$F,$C);}catch(Exception$Id){$this->error=$Id->getMessage();return
false;}$this->version=preg_replace('~^\D*([\d.]+).*~',"$1",(string)@$this->pdo->getAttribute(PDO::ATTR_SERVER_VERSION));return
true;}function
quote($yl){return$this->pdo->quote($yl);}function
query($I,$Nm=false){$ul=$this->pdo->query($I);$this->error="";if(!$ul)return$this->storeError(false);$J=new
PdoResult($ul);$this->storeResult($J);return$J;}function
storeResult($J=null){if(!$J){$J=$this->multiResult;if(!$J)return
false;}if($J->getColumnsCount())return$J;$this->affectedRows=$J->getAffectedRowsCount();return
true;}function
nextResult(){$this->error="";if(!$this->multiResult)return
false;if($this->multiResult->nextRowset())return
true;$Cd=$this->multiResult->getErrorInfo();$this->multiResult=false;if(!isset($Cd[2]))return
false;$this->storeErrorInfo($Cd);return
true;}function
inTransaction(){return$this->pdo->inTransaction();}function
begin(){return$this->storeError($this->pdo->beginTransaction());}function
commit(){return!$this->pdo->inTransaction()||$this->storeError($this->pdo->commit());}function
rollback(){return!$this->pdo->inTransaction()||$this->storeError($this->pdo->rollBack());}private
function
storeError($K){if(!$K)$this->storeErrorInfo($this->pdo->errorInfo());return$K;}protected
function
storeErrorInfo(array$Cd){list(,$Bd,$j)=$Cd;$this->errno=(int)$Bd;$this->error=$j?:lang(123);}}class
PdoResult
extends
Result{private$statement;private$offset=0;function
__construct(PDOStatement$ul){parent::__construct(max($ul->columnCount()?$ul->rowCount():0,0));$this->statement=$ul;}function
getColumnsCount(){return$this->statement->columnCount();}function
getAffectedRowsCount(){return$this->statement->rowCount();}function
fetchAssoc(){return$this->fetchArray(PDO::FETCH_ASSOC);}function
fetchRow(){return$this->fetchArray(PDO::FETCH_NUM);}private
function
fetchArray($yh){$J=$this->statement->fetch($yh);return$J?array_map([$this,'unresource'],$J):$J;}private
function
unresource($Y){return
is_resource($Y)?stream_get_contents($Y):$Y;}function
fetchField(){$L=$this->statement->getColumnMeta($this->offset++);if($L===false)return
false;$U=$L["pdo_type"];$L["type"]=($U==PDO::PARAM_INT?0:15);$L["charsetnr"]=($U==\PDO::PARAM_LOB||(isset($L["flags"])&&in_array("blob",(array)$L["flags"]))?63:0);return(object)$L;}function
seek($di){for($q=0;$q<$di;$q++){if($this->statement->fetch()===false)return
false;;}return
true;}function
nextRowset(){$this->offset=0;return@$this->statement->nextRowset();}function
getErrorInfo(){return$this->statement->errorInfo();}}}class
Drivers{private
static$drivers=[];private
static$extensions=[];static
function
add($r,$A,array$Sd){self::$drivers[$r]=$A;self::$extensions[$r]=$Sd;}static
function
setName($r,$A){if(isset(self::$drivers[$r]))self::$drivers[$r]=$A;}static
function
get($r){return
isset(self::$drivers[$r])?self::$drivers[$r]:null;}static
function
getList(){return
self::$drivers;}static
function
getExtensions($r){return
isset(self::$extensions[$r])?self::$extensions[$r]:[];}}function
get_drivers(){return
Drivers::getList();}abstract
class
Driver{static$EnumLengthPattern="'(?:''|[^'\\\\]|\\\\.)*'";protected$connection;protected$admin;protected$types=[];private$userTypes=[];protected$unsigned=[];protected$generated=[];protected$operators=[];protected$likeOperator="LIKE %%";protected$functions=[];protected$grouping=[];protected$inOut=["IN","OUT","INOUT"];protected$onActions=["RESTRICT","CASCADE","SET NULL","SET DEFAULT","NO ACTION"];protected$partitionBy=[];protected$insertFunctions=[];protected$editFunctions=[];protected$systemDatabases=[];protected$systemSchemas=[];private
static$instance=null;static
function
create(Connection$d,$ya){if(self::$instance)die(__CLASS__." instance already exists.\n");return
self::$instance=new
static($d,$ya);}static
function
get(){if(!self::$instance)exit(__CLASS__." instance not found.\n");return
self::$instance;}protected
function
__construct(Connection$d,$ya){$this->connection=$d;$this->admin=$ya;}function
getTypes(){return
call_user_func_array("array_merge",array_values($this->types));}function
getStructuredTypes(){return
array_map("array_keys",$this->types);}function
setUserTypes(array$Mm){$this->userTypes=array_flip($Mm);$this->types[lang(110)]=array_fill_keys(array_keys($this->userTypes),0);}function
getUserTypes(){return
array_keys($this->userTypes);}function
getUserTypeId($A){return
isset($this->userTypes[$A])?$this->userTypes[$A]:null;}function
getUnsigned(){return$this->unsigned;}function
getGenerated(){return$this->generated;}function
getOperators(){return$this->operators;}function
getLikeOperator(){return$this->likeOperator;}function
getFunctions(){return$this->functions;}function
getGrouping(){return$this->grouping;}function
getInOut(){return$this->inOut;}function
getOnActions(){return$this->onActions;}function
getPartitionBy(){return$this->partitionBy;}function
getInsertFunctions(){return$this->insertFunctions;}function
getEditFunctions(){return$this->editFunctions;}function
getSystemDatabases(){return$this->systemDatabases;}function
getSystemSchemas(){return$this->systemSchemas;}function
getUnconvertFunction(array$k){return"";}function
select($Q,array$N,array$Z,array$Ne,array$D=[],$w=1,$E=0,$Bj=false){$Wf=(count($Ne)<count($N));$I="SELECT".limit(($_GET["page"]!="last"&&$w&&$Ne&&$Wf&&DIALECT=="sql"?"SQL_CALC_FOUND_ROWS ":"").implode(", ",$N)."\nFROM ".table($Q),($Z?"\nWHERE ".implode(" AND ",$Z):"").($Ne&&$Wf?"\nGROUP BY ".implode(", ",$Ne):"").($D?"\nORDER BY ".implode(", ",$D):""),$w,($E?$w*$E:0),"\n");$tl=microtime(true);$K=$this->connection->query($I);if($Bj)echo
Admin::get()->formatSelectQuery($I,$tl,!$K);return$K;}function
delete($Q,$Mj,$w=0){$I="FROM ".table($Q);return
queries("DELETE".($w?limit1($Q,$I,$Mj):" $I$Mj"));}function
update($Q,array$Sj,$Mj,$w=0,$Nk="\n"){$nn=[];foreach($Sj
as$u=>$X)$nn[]="$u = $X";$I=table($Q)." SET$Nk".implode(",$Nk",$nn);return
queries("UPDATE".($w?limit1($Q,$I,$Mj,$Nk):" $I$Mj"));}function
insert($Q,array$Sj){return
queries("INSERT INTO ".table($Q).($Sj?" (".implode(", ",array_keys($Sj)).")\nVALUES (".implode(", ",$Sj).")":" DEFAULT VALUES").$this->getInsertReturningSql($Q));}function
getInsertReturningSql($Q){return"";}function
insertUpdate($Q,array$Tj,array$Aj){return
false;}function
begin(){remember_query("BEGIN");return$this->connection->begin();}function
commit(){remember_query("COMMIT");return$this->connection->commit();}function
rollback(){remember_query("ROLLBACK");return$this->connection->rollback();}function
slowQuery($I,$rm){return
null;}function
convertSearch($qf,array$Z,array$k){return$qf;}function
isSearchable(array$Z,array$k){return
true;}function
getNull(){return"NULL";}function
getTypeName(stdClass$k){return
isset($k->native_type)?$k->native_type:"";}function
quoteBinary($yl){return
q($yl);}function
warnings(){return
null;}function
tableHelp($A,$Uf=false){return
null;}function
supportsIndex(array$Rl){return!is_view($Rl);}function
supportsAlterIndex(array$Rl){return
true;}function
getIndexAlgorithms(array$Rl){return[];}function
getIndexOpclasses(){return[];}function
getInheritedTables($Q){return[];}function
getParentTables($Q){return[];}function
isPartition($Q){return
false;}function
getPartitionsInfo($Q){return[];}function
hasCStyleEscapes(){return
false;}function
lineComment(){return"--";}function
engines(){return[];}function
explodeArrayValue($Y,$U,&$wk){return[];}function
implodeArrayValues(array$nn,$U){return"";}function
checkConstraints($Q){return
get_key_vals("SELECT c.CONSTRAINT_NAME, CHECK_CLAUSE
FROM INFORMATION_SCHEMA.CHECK_CONSTRAINTS c
JOIN INFORMATION_SCHEMA.TABLE_CONSTRAINTS t ON c.CONSTRAINT_SCHEMA = t.CONSTRAINT_SCHEMA
	AND c.CONSTRAINT_NAME = t.CONSTRAINT_NAME".($this->connection->isMariaDB()?" AND c.TABLE_NAME = ".q($Q):"")."
WHERE c.CONSTRAINT_SCHEMA = ".($_GET["ns"]!=""?q($_GET["ns"]):(DIALECT=="pgsql"?"current_schema()":"DATABASE()"))."
AND t.TABLE_NAME = ".q($Q).(DIALECT=="pgsql"?"
AND CHECK_CLAUSE NOT LIKE '% IS NOT NULL'":""),$this->connection);}function
getAllFields(){if(DB=="")return[];$Ca=[];$fg=(DIALECT=="pgsql"||DIALECT=="mssql");$Aj=(DIALECT=="sql"?"c.COLUMN_KEY = 'PRI'":($fg?"k.COLUMN_NAME":""));$M=get_rows("SELECT c.TABLE_NAME AS tab, c.COLUMN_NAME AS field, c.IS_NULLABLE AS nullable,
	c.DATA_TYPE AS type, c.CHARACTER_MAXIMUM_LENGTH AS length".($Aj?",
	$Aj AS ".idf_escape("primary"):"")."
FROM INFORMATION_SCHEMA.COLUMNS c".($fg?"
LEFT JOIN INFORMATION_SCHEMA.TABLE_CONSTRAINTS t
	ON c.TABLE_SCHEMA = t.TABLE_SCHEMA AND c.TABLE_NAME = t.TABLE_NAME AND t.CONSTRAINT_TYPE = 'PRIMARY KEY'
LEFT JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE k
	ON t.CONSTRAINT_SCHEMA = k.CONSTRAINT_SCHEMA AND t.CONSTRAINT_NAME = k.CONSTRAINT_NAME
		AND c.TABLE_SCHEMA = k.TABLE_SCHEMA AND c.TABLE_NAME = k.TABLE_NAME AND c.COLUMN_NAME = k.COLUMN_NAME":"")."
WHERE c.TABLE_SCHEMA = ".q($_GET["ns"]!=""?$_GET["ns"]:DB)."
ORDER BY c.TABLE_NAME, c.ORDINAL_POSITION",$this->connection);foreach($M
as$L){$L["null"]=($L["nullable"]=="YES");$Ca[$L["tab"]][]=$L;}return$Ca;}}Drivers::add("mysql","MySQL",["MySQLi","PDO_MySQL"]);if(isset($_GET["mysql"])&&!defined("AdminNeo\DRIVER")){define("AdminNeo\DRIVER","mysql");define("AdminNeo\DIALECT","sql");if(extension_loaded("mysqli")&&$_GET["ext"]!="pdo"){define("AdminNeo\DRIVER_EXTENSION","MySQLi");class
MySqlConnection
extends
Connection{private$mysqli;private$readOnly=false;protected
function
__construct(){parent::__construct();$this->mysqli=new
mysqli();$this->mysqli->init();}function
getDefaultServerName(){return"localhost";}function
open($O,$V,$F){mysqli_report(MYSQLI_REPORT_OFF);list($jf,$sj)=host_port($O);$u=Admin::get()->getConfig()->getSslKey();$nb=Admin::get()->getConfig()->getSslCertificate();$lb=Admin::get()->getConfig()->getSslCaCertificate();$sl=$u||$nb||$lb;if($sl){$this->mysqli->ssl_set($u,$nb,$lb,null,null);$ne=Admin::get()->getConfig()->getSslTrustServerCertificate()?64:MYSQLI_CLIENT_SSL;}else$ne=0;$Xb=@$this->mysqli->real_connect(($O!=""?$jf:ini_get("mysqli.default_host")),($O.$V!=""?$V:ini_get("mysqli.default_user")),($O.$V.$F!=""?$F:ini_get("mysqli.default_pw")),null,(is_numeric($sj)?(int)$sj:ini_get("mysqli.default_port")),(!is_numeric($sj)?$sj:null),$ne);$this->mysqli->options(MYSQLI_OPT_LOCAL_INFILE,false);if($Xb){$_f=$this->mysqli->get_server_info();$this->version=str_replace("-MariaDB","",$_f);$this->flavor=str_contains($_f,"MariaDB")?"mariadb":null;}return$Xb;}function
getAffectedRows(){return$this->mysqli->affected_rows;}function
getErrno(){return$this->mysqli->errno;}function
getError(){return$this->mysqli->error;}function
selectDatabase($A){return$this->mysqli->select_db($A);}function
setCharset($rb){if($this->mysqli->set_charset($rb))return
true;$this->mysqli->set_charset('utf8');return(bool)$this->query("SET NAMES $rb");}function
quote($yl){return"'".$this->mysqli->escape_string($yl)."'";}function
query($I,$Nm=false){if($this->untrusted&&!$this->readOnly){$this->readOnly=(bool)$this->mysqli->query("SET SESSION TRANSACTION READ ONLY");if(!$this->readOnly)return
false;}$J=$this->mysqli->query($I,($Nm?MYSQLI_USE_RESULT:MYSQLI_STORE_RESULT));return
is_object($J)?new
MySqlResult($J):$J;}function
getQueryInfo(){return$this->mysqli->info;}function
getWarningsCount(){return$this->mysqli->warning_count;}function
multiQuery($I){return$this->mysqli->multi_query($I);}function
storeResult($J=null){$J=$this->mysqli->store_result();if(!$J)return
false;return
new
MySqlResult($J);}function
hasMoreResults(){return$this->mysqli->more_results();}function
nextResult(){return$this->mysqli->more_results()&&($this->mysqli->next_result()||$this->mysqli->errno);}}class
MySqlResult
extends
Result{private$resource;function
__construct(mysqli_result$hk){parent::__construct($hk->num_rows);$this->resource=$hk;}function
fetchAssoc(){return$this->resource->fetch_assoc();}function
fetchRow(){return$this->resource->fetch_row();}function
fetchField(){return$this->resource->fetch_field();}function
seek($di){return$this->resource->data_seek($di);}}}elseif(extension_loaded("pdo_mysql")){define("AdminNeo\DRIVER_EXTENSION","PDO_MySQL");class
MySqlConnection
extends
PdoConnection{private$readOnly=false;function
getDefaultServerName(){return"localhost";}function
open($O,$V,$F){list($jf,$sj)=host_port($O);$id="mysql:charset=utf8".($jf!=""?";host=$jf":"").($sj?(is_numeric($sj)?";port=":";unix_socket=").$sj:"");$C=[PDO::MYSQL_ATTR_LOCAL_INFILE=>false];if(isset($_GET["select"])&&defined('\PDO::MYSQL_ATTR_MULTI_STATEMENTS'))$C[PDO::MYSQL_ATTR_MULTI_STATEMENTS]=false;$u=Admin::get()->getConfig()->getSslKey();if($u)$C[PDO::MYSQL_ATTR_SSL_KEY]=$u;$nb=Admin::get()->getConfig()->getSslCertificate();if($nb)$C[PDO::MYSQL_ATTR_SSL_CERT]=$nb;$lb=Admin::get()->getConfig()->getSslCaCertificate();if($lb)$C[PDO::MYSQL_ATTR_SSL_CA]=$lb;$Hm=Admin::get()->getConfig()->getSslTrustServerCertificate();if($Hm!==null&&defined('\PDO::MYSQL_ATTR_SSL_VERIFY_SERVER_CERT'))$C[PDO::MYSQL_ATTR_SSL_VERIFY_SERVER_CERT]=!$Hm;if(!$this->dsn($id,$V,$F,$C))return
false;$rn=@$this->pdo->getAttribute(PDO::ATTR_SERVER_VERSION);$this->flavor=str_contains($rn,"MariaDB")?"mariadb":null;return
true;}function
setCharset($rb){return(bool)$this->query("SET NAMES $rb");}function
selectDatabase($A){return(bool)$this->query("USE ".idf_escape($A));}function
query($I,$Nm=false){if($this->untrusted&&!$this->readOnly){$this->readOnly=(bool)parent::query("SET SESSION TRANSACTION READ ONLY");if(!$this->readOnly)return
false;}$this->pdo->setAttribute(PDO::MYSQL_ATTR_USE_BUFFERED_QUERY,!$Nm);return
parent::query($I,$Nm);}}}class
MySqlDriver
extends
Driver{protected
function
__construct(Connection$d,$ya){parent::__construct($d,$ya);$this->types=[lang(124)=>["tinyint"=>3,"smallint"=>5,"mediumint"=>8,"int"=>10,"bigint"=>20,"decimal"=>66,"float"=>12,"double"=>21,],lang(125)=>["date"=>10,"datetime"=>19,"timestamp"=>19,"time"=>10,"year"=>4,],lang(126)=>["char"=>255,"varchar"=>65535,"tinytext"=>255,"text"=>65535,"mediumtext"=>16777215,"longtext"=>4294967295,],lang(127)=>["enum"=>65535,"set"=>64,],lang(128)=>["bit"=>20,"binary"=>255,"varbinary"=>65535,"tinyblob"=>255,"blob"=>65535,"mediumblob"=>16777215,"longblob"=>4294967295,],lang(129)=>["geometry"=>0,"point"=>0,"linestring"=>0,"polygon"=>0,"multipoint"=>0,"multilinestring"=>0,"multipolygon"=>0,"geometrycollection"=>0,],];$this->unsigned=["unsigned","zerofill","unsigned zerofill"];$Wg=$d->isMariaDB();if($d->isMinVersion($Wg?"10.2":"5.7"))$this->generated=["STORED","VIRTUAL"];$this->operators=["=","<",">","<=",">=","!=","LIKE","LIKE %%","NOT LIKE","IN","NOT IN","FIND_IN_SET","IS NULL","IS NOT NULL","REGEXP","NOT REGEXP","SQL",];$this->functions=["char_length","lower","upper","round","floor","ceil","date","from_unixtime","unix_timestamp","sec_to_time","time_to_sec",];$this->grouping=["sum","min","max","avg","count","count distinct","group_concat",];$this->partitionBy=["RANGE","LIST","HASH","LINEAR HASH","KEY","LINEAR KEY"];$this->insertFunctions=["char"=>"md5/sha1/password/encrypt/uuid","binary"=>"md5/sha1","date|time"=>"now",];$this->editFunctions=[number_type()=>"+/-","date"=>"+ interval/- interval","time"=>"addtime/subtime","char|text"=>"concat",];if($d->isMinVersion($Wg?"10.2":"5.7.8"))$this->types[lang(126)]["json"]=4294967295;if($Wg&&$d->isMinVersion("10.7")){$this->types[lang(126)]["uuid"]=128;$this->insertFunctions['uuid']='uuid';}if($Wg&&$d->isMinVersion("10.5")){$this->types[lang(130)]["inet6"]=39;if($d->isMinVersion("10.10"))$this->types[lang(130)]["inet4"]=15;}if($d->isMinVersion($Wg?"11.7":"9"))$this->types[lang(124)]["vector"]=16383;$this->systemDatabases=["mysql","information_schema","performance_schema","sys"];}function
insert($Q,array$Sj){return($Sj?parent::insert($Q,$Sj):queries("INSERT INTO ".table($Q)." ()\nVALUES ()"));}function
getUnconvertFunction(array$k){if(preg_match("~binary~",$k["type"]))return"<code class='jush-sql'>UNHEX</code>";elseif($k["type"]=="bit")return
doc_link(['sql'=>'bit-value-literals.html','mariadb'=>"reference/sql-structure/sql-language-structure/binary-literals"],"<code>b''</code>");elseif($k["type"]=="vector")return"<code class='jush-sql'>".($this->connection->isMariaDB()?"VEC_FromText":"STRING_TO_VECTOR")."</code>";elseif(preg_match("~geometry|point|linestring|polygon~",$k["type"]))return"<code class='jush-sql'>GeomFromText</code>";else
return"";}function
getTypeName(stdClass$k){$Mm=["decimal","tinyint","smallint","int","float","double",7=>"timestamp","bigint","mediumint","date","time","datetime","year",15=>"varchar","bit",242=>"vector",245=>"json","decimal","enum","set","tinytext","mediumtext","longtext","text","varchar","char","geometry",];$U=isset($Mm[$k->type])?$Mm[$k->type]:"";return
parent::getTypeName($k)?:($k->charsetnr==63?str_replace(["text","varchar","char"],["blob","varbinary","binary"],$U):$U);}function
quoteBinary($yl){return"X".q(bin2hex($yl));}function
insertUpdate($Q,array$Tj,array$Aj){$c=array_keys(reset($Tj));$yj="INSERT INTO ".table($Q)." (".implode(", ",$c).") VALUES\n";$nn=[];foreach($c
as$u)$nn[$u]="$u = VALUES($u)";$Fl="\nON DUPLICATE KEY UPDATE ".implode(", ",$nn);$nn=[];$v=0;foreach($Tj
as$Sj){$Y="(".implode(", ",$Sj).")";if($nn&&(strlen($yj)+$v+strlen($Y)+strlen($Fl)>1e6)){if(!queries($yj.implode(",\n",$nn).$Fl))return
false;$nn=[];$v=0;}$nn[]=$Y;$v+=strlen($Y)+2;}return
queries($yj.implode(",\n",$nn).$Fl);}function
slowQuery($I,$rm){$Wg=$this->connection->isMariaDB();if(!$this->connection->isMinVersion($Wg?"10.1.2":"5.7.8"))return
null;if($Wg)return"SET STATEMENT max_statement_time=$rm FOR $I";elseif(preg_match('~^(SELECT\b)(.+)~is',$I,$z))return"$z[1] /*+ MAX_EXECUTION_TIME(".($rm*1000).") */ $z[2]";else
return
null;}function
convertSearch($qf,array$Z,array$k){return(preg_match('~char|text|enum|set~',$k["type"])&&!preg_match("~^utf8~",$k["collation"])&&preg_match('~[\x80-\xFF]~',$Z['val'])?"CONVERT($qf USING ".charset($this->connection).")":$qf);}function
isSearchable(array$Z,array$k){return$k["type"]!="bit"||preg_match('~IN$|NULL$|^(FIND_IN_SET|SQL)$~',$Z["op"])||($Z["op"]!="LIKE %%"&&preg_match('~^[01]+$~',$Z["val"]));}function
warnings(){$J=$this->connection->query("SHOW WARNINGS");if($J&&$J->getRowsCount()){ob_start();print_select_result($J);return
ob_get_clean();}return
null;}function
tableHelp($A,$Uf=false){$Wg=$this->connection->isMariaDB();if(DB=="information_schema"){$A=strtolower($A);return$Wg?"reference/system-tables/information-schema/information-schema-tables/".(str_starts_with($A,"innodb_")?"information-schema-innodb-tables/":"")."information-schema-$A-table":"information-schema-".str_replace("_","-",$A)."-table.html";}if(DB=="performance_schema")return$Wg?"reference/system-tables/performance-schema/performance-schema-tables/performance-schema-$A-table":"performance-schema-".str_replace("_","-",$A)."-table.html";if(DB=="sys"){if($Wg)return"reference/system-tables/sys-schema/";return"sys-".strtolower(str_replace("_","-",preg_replace('~^x\$~','',$A))).".html";}if(DB=="mysql")return$Wg?"reference/system-tables/the-mysql-database-tables/mysql-$A".str_starts_with($A,"innodb_")?"":"-table":"system-schema.html";return
null;}function
getPartitionsInfo($Q){$Be="FROM information_schema.PARTITIONS WHERE TABLE_SCHEMA = ".q(DB)." AND TABLE_NAME = ".q($Q);$J=Connection::get()->query("SELECT PARTITION_METHOD, PARTITION_EXPRESSION, PARTITION_ORDINAL_POSITION $Be ORDER BY PARTITION_ORDINAL_POSITION DESC LIMIT 1")->fetchRow();if(!$J)return[];$_f=["partition_by"=>$J[0],"partition"=>$J[1],"partitions"=>$J[2],];$cj=get_key_vals("SELECT PARTITION_NAME, PARTITION_DESCRIPTION $Be AND PARTITION_NAME != '' ORDER BY PARTITION_ORDINAL_POSITION");$_f["partition_names"]=array_keys($cj);$_f["partition_values"]=array_values($cj);return$_f;}function
getIndexAlgorithms(array$Rl){return
preg_match('~^(MEMORY|NDB)$~',$Rl["Engine"])?["BTREE","HASH"]:["BTREE"];}function
hasCStyleEscapes(){static$jb;if($jb===null){$ql=$this->connection->getValue("SHOW VARIABLES LIKE 'sql_mode'",1);$jb=(strpos($ql,'NO_BACKSLASH_ESCAPES')===false);}return$jb;}function
lineComment(){return"#|-- ";}function
engines(){$zd=[];foreach(get_rows("SHOW ENGINES")as$L){if(preg_match("~YES|DEFAULT~",$L["Support"]))$zd[]=$L["Engine"];}return$zd;}}function
create_driver(Connection$d){return
MySqlDriver::create($d,Admin::get());}function
idf_escape($qf){return"`".str_replace("`","``",$qf)."`";}function
table($qf){return
idf_escape($qf);}function
connect($Aj=false,&$j=null){$d=$Aj?MySqlConnection::create():MySqlConnection::createSecondary();list($O,$V,$F)=Admin::get()->getCredentials();if(!$d->openPasswordless($O,$V,$F,false)){$j=$d->getError();if(function_exists('iconv')&&!is_utf8($j)&&strlen($sk=iconv("windows-1252","utf-8//IGNORE",$j))>strlen($j))$j=$sk;return
null;}$d->setCharset(charset($d));$d->query("SET sql_quote_show_create = 1, autocommit = 1");if($Aj&&$d->isMariaDB()){Drivers::setName(DRIVER,"MariaDB");save_driver_name(DRIVER,$O,"MariaDB");}return$d;}function
get_databases($pe){$g=get_session("dbs");if($g===null){$I="SELECT SCHEMA_NAME FROM information_schema.SCHEMATA ORDER BY SCHEMA_NAME";$tl=microtime(true);$g=($pe?slow_query($I):get_vals($I));if(microtime(true)-$tl>0.1){restart_session();set_session("dbs",$g);stop_session();}}return$g;}function
limit($I,$Z,$w,$di=0,$Nk=" "){return" $I$Z".($w?$Nk."LIMIT $w".($di?" OFFSET $di":""):"");}function
limit1($Q,$I,$Z,$Nk="\n"){return
limit($I,$Z,1,0,$Nk);}function
db_collation($h,array$Fb){$K=null;$e=Connection::get()->getValue("SHOW CREATE DATABASE ".idf_escape($h),1);if(preg_match('~ COLLATE ([^ ]+)~',$e,$z))$K=$z[1];elseif(preg_match('~ CHARACTER SET ([^ ]+)~',$e,$z))$K=$Fb[$z[1]][-1];return$K;}function
logged_user(){return
Connection::get()->getValue("SELECT USER()");}function
tables_list(){return
get_key_vals("SELECT TABLE_NAME, TABLE_TYPE FROM information_schema.TABLES WHERE TABLE_SCHEMA = DATABASE() ORDER BY TABLE_NAME");}function
count_tables(array$g){$K=[];foreach($g
as$h)$K[$h]=count(get_vals("SHOW TABLES IN ".idf_escape($h)));return$K;}function
table_status($A="",$Xd=false){if($Xd)$I="SELECT TABLE_NAME AS Name, ENGINE AS Engine, CREATE_OPTIONS AS Create_options,
	TABLES.TABLE_COLLATION AS Collation, TABLE_COMMENT AS Comment
FROM information_schema.TABLES
WHERE TABLE_SCHEMA = DATABASE() ".($A!=""?"AND TABLE_NAME = ".q($A):"ORDER BY Name");else$I="SHOW TABLE STATUS".($A!=""?" LIKE ".q(addcslashes($A,"%_\\")):"");$S=[];foreach(get_rows($I)as$L){if($L["Engine"]=="InnoDB")$L["Comment"]=preg_replace('~(?:(.+); )?InnoDB free: .*~','\1',$L["Comment"]);if(!isset($L["Engine"]))$L["Comment"]="";if($A!="")$L["Name"]=$A;$S[$L["Name"]]=$L;}return$S;}function
is_view(array$R){return$R["Engine"]===null;}function
fk_support(array$R){return
preg_match('~InnoDB|IBMDB2I'.(Connection::get()->isMinVersion("5.6")?'|NDB':'').'~i',$R["Engine"]);}function
parse_type($De){preg_match('~^([^( ]+)(?:\((.+)\))?( unsigned)?( zerofill)?$~',$De,$_);return[$_[1],isset($_[2])?$_[2]:"",ltrim((isset($_[3])?$_[3]:"").(isset($_[4])?$_[4]:""))];}function
fields($Q){$Wg=Connection::get()->isMariaDB();$K=[];foreach(get_rows("SELECT * FROM information_schema.COLUMNS WHERE TABLE_SCHEMA = DATABASE() AND TABLE_NAME = ".q($Q)." ORDER BY ORDINAL_POSITION")as$L){$k=$L["COLUMN_NAME"];$U=preg_replace('~\s?/\*.+\*/~U',"",$L["COLUMN_TYPE"]);$Td=$L["EXTRA"];preg_match('~^(VIRTUAL|PERSISTENT|STORED)~',$Td,$Ge);list($Lm,$v,$Vm)=parse_type($U);$i=$Wg&&$L["COLUMN_DEFAULT"]=="NULL"?null:$L["COLUMN_DEFAULT"];if($i!==null){$ag=preg_match('~(text|json)~',$Lm);if(!$Wg&&$ag)$i=preg_replace("~^(_\w+)?('.*')$~",'\2',stripslashes($i));if($Wg||$ag){$i=preg_replace_callback("~^'(.*)'$~",function($_){return
stripslashes(str_replace("''","'",$_[1]));},$i);}if(!$Wg&&preg_match('~binary~',$Lm)&&preg_match('~^0x(\w*)$~',$i,$_))$i=pack("H*",$_[1]);}$Ie=$L["GENERATION_EXPRESSION"];if(!$Wg)$Ie=preg_replace("~(^|,|\()(_\w+)?('.*')($|,|\))~",'\1\3\4',stripslashes($Ie));$K[$k]=["field"=>$k,"full_type"=>$U,"type"=>$Lm,"length"=>$v,"unsigned"=>$Vm,"default"=>($Ge?$Ie:$i),"null"=>($L["IS_NULLABLE"]=="YES"),"auto_increment"=>($Td=="auto_increment"),"on_update"=>(preg_match('~\bon update (\w+)~i',$Td,$_)?$_[1]:""),"collation"=>$L["COLLATION_NAME"],"privileges"=>array_flip(explode(",",$L["PRIVILEGES"]))+["where"=>1,"order"=>1],"comment"=>$L["COLUMN_COMMENT"],"primary"=>($L["COLUMN_KEY"]=="PRI"),"generated"=>($Ge[1]=="PERSISTENT"?"STORED":$Ge[1]),"srid"=>isset($L["SRS_ID"])?$L["SRS_ID"]:null,];}return$K;}function
indexes($Q,$d=null){$K=[];foreach(get_rows("SHOW INDEX FROM ".table($Q),$d)as$L){$A=$L["Key_name"];$K[$A]["type"]=($A=="PRIMARY"?"PRIMARY":($L["Index_type"]=="FULLTEXT"?"FULLTEXT":($L["Non_unique"]?(preg_match('~^(SPATIAL|VECTOR)$~',$L["Index_type"])?$L["Index_type"]:"INDEX"):"UNIQUE")));$K[$A]["columns"][]=$L["Column_name"];$K[$A]["lengths"][]=($L["Index_type"]=="SPATIAL"?null:$L["Sub_part"]);$K[$A]["descs"][]=($L["Collation"]=="D"?'1':null);$K[$A]["algorithm"]=$L["Index_type"];}return$K;}function
foreign_keys($Q){static$ij='(?:`(?:[^`]|``)+`|"(?:[^"]|"")+")';$K=[];$lc=Connection::get()->getValue("SHOW CREATE TABLE ".table($Q),1);if($lc){$ni=implode("|",Driver::get()->getOnActions());preg_match_all("~CONSTRAINT ($ij) FOREIGN KEY ?\\(((?:$ij,? ?)+)\\) REFERENCES ($ij)(?:\\.($ij))? "."\\(((?:$ij,? ?)+)\\)(?: ON DELETE ($ni))?(?: ON UPDATE ($ni))?~",$lc,$_,PREG_SET_ORDER);foreach($_
as$z){preg_match_all("~$ij~",$z[2],$kl);preg_match_all("~$ij~",$z[5],$gm);$K[idf_unescape($z[1])]=["db"=>idf_unescape($z[4]!=""?$z[3]:$z[4]),"table"=>idf_unescape($z[4]!=""?$z[4]:$z[3]),"source"=>array_map('AdminNeo\idf_unescape',$kl[0]),"target"=>array_map('AdminNeo\idf_unescape',$gm[0]),"on_delete"=>($z[6]?:"RESTRICT"),"on_update"=>($z[7]?:"RESTRICT"),];}}return$K;}function
backward_keys($Q){$I="SELECT CONSTRAINT_NAME AS constraint_name, TABLE_SCHEMA AS table_schema, TABLE_NAME AS table_name,
COLUMN_NAME AS column_name, REFERENCED_COLUMN_NAME AS referenced_column_name
FROM information_schema.KEY_COLUMN_USAGE
WHERE TABLE_SCHEMA = ".q(Admin::get()->getDatabase())."
AND REFERENCED_TABLE_SCHEMA = ".q(Admin::get()->getDatabase())."
AND REFERENCED_TABLE_NAME = ".q($Q)."
ORDER BY ORDINAL_POSITION";return
get_rows($I,null,"");}function
view($A){$N=Connection::get()->getValue("SHOW CREATE VIEW ".table($A),1);$Tg='(?:[^`\']|`[^`]*`|\'[^\']*\')*';$N=preg_replace("~^$Tg\\s+AS\\s+~isU","",$N);return["select"=>format_sql($N)];}function
collations(){$K=[];$I=Connection::get()->isMariaDB()&&Connection::get()->isMinVersion("10.10")?"SELECT CHARACTER_SET_NAME AS Charset, FULL_COLLATION_NAME AS Collation, IS_DEFAULT AS `Default` FROM information_schema.COLLATION_CHARACTER_SET_APPLICABILITY":"SHOW COLLATION";foreach(get_rows($I)as$L){if($L["Default"])$K[$L["Charset"]][-1]=$L["Collation"];else$K[$L["Charset"]][]=$L["Collation"];}ksort($K);foreach($K
as$u=>$X)sort($K[$u]);return$K;}function
information_schema($h,$_k=""){return($h=="information_schema")||(Connection::get()->isMinVersion("5.5")&&$h=="performance_schema");}function
error(){return
h(preg_replace('~^You have an error.*syntax to use~U',"Syntax error",Connection::get()->getError()));}function
create_database($h,$Eb){return(bool)queries("CREATE DATABASE ".idf_escape($h).($Eb?" COLLATE ".q($Eb):""));}function
drop_databases(array$g){$K=apply_queries("DROP DATABASE",$g,'AdminNeo\idf_escape');restart_session();set_session("dbs",null);return$K;}function
rename_database($A,$Eb){$K=false;if(create_database($A,$Eb)){$S=[];$un=[];foreach(tables_list()as$Q=>$U){if($U=='VIEW')$un[]=$Q;else$S[]=$Q;}$K=(!$S&&!$un)||move_tables($S,$un,$A);drop_databases($K?[DB]:[]);}return$K;}function
auto_increment(){$Ra=" PRIMARY KEY";if($_GET["create"]!=""&&$_POST["auto_increment_col"]){foreach(indexes($_GET["create"])as$s){if(in_array($_POST["fields"][$_POST["auto_increment_col"]]["orig"],$s["columns"],true)){$Ra="";break;}if($s["type"]=="PRIMARY")$Ra=" UNIQUE";}}return" AUTO_INCREMENT$Ra";}function
alter_table($Q,$A,array$l,array$re,$Ob,$yd,$Eb,$Qa,$bj){$Ha=[];foreach($l
as$k){if($k[1]){$i=$k[1][3];if(str_contains($i," GENERATED")){$k[1][3]=Connection::get()->isMariaDB()?"":$k[1][2];$k[1][2]=$i;}$Ha[]=($Q!=""?($k[0]!=""?"CHANGE ".idf_escape($k[0]):"ADD"):" ")." ".implode($k[1]).($Q!=""?$k[2]:"");}else$Ha[]="DROP ".idf_escape($k[0]);}$Ha=array_merge($Ha,$re);$vl=($Ob!==null?" COMMENT=".q($Ob):"").($yd?" ENGINE=".q($yd):"").($Eb?" COLLATE ".q($Eb):"").($Qa!=""?" AUTO_INCREMENT=$Qa":"");if($bj){$cj=[];if($bj["partition_by"]=='RANGE'||$bj["partition_by"]=='LIST'){foreach($bj["partition_names"]as$u=>$X){$Y=$bj["partition_values"][$u];$cj[]="\n  PARTITION ".idf_escape($X)." VALUES ".($bj["partition_by"]=='RANGE'?"LESS THAN":"IN").($Y!=""?" ($Y)":" MAXVALUE");}}$vl
.="\nPARTITION BY {$bj["partition_by"]}({$bj["partition"]})";if($cj)$vl
.=" (".implode(",",$cj)."\n)";elseif($bj["partitions"])$vl
.=" PARTITIONS ".(int)$bj["partitions"];}elseif($bj===null)$vl
.="\nREMOVE PARTITIONING";if($Q=="")return(bool)queries("CREATE TABLE ".table($A)." (\n".implode(",\n",$Ha)."\n)$vl");if($Q!=$A)$Ha[]="RENAME TO ".table($A);if($vl)$Ha[]=ltrim($vl);return!$Ha||queries("ALTER TABLE ".table($Q)."\n".implode(",\n",$Ha));}function
alter_indexes($Q,array$Ha){$pb=[];foreach($Ha
as$u=>$X)$pb[]=($X[2]=="DROP"?"\nDROP INDEX ".idf_escape($X[1]):"\nADD $X[0] ".($X[0]=="PRIMARY"?"KEY ":"").($X[1]!=""?idf_escape($X[1])." ":"")."(".implode(", ",$X[2]).")");return(bool)queries("ALTER TABLE ".table($Q).implode(",",$pb));}function
truncate_tables(array$S){return
apply_queries("TRUNCATE TABLE",$S);}function
drop_views(array$un){return(bool)queries("DROP VIEW ".implode(", ",array_map('AdminNeo\table',$un)));}function
drop_tables(array$S){return(bool)queries("DROP TABLE ".implode(", ",array_map('AdminNeo\table',$S)));}function
move_tables(array$S,array$un,$gm){$ek=[];foreach($S
as$Q)$ek[]=table($Q)." TO ".idf_escape($gm).".".table($Q);if(!$ek||queries("RENAME TABLE ".implode(", ",$ek))){$Gc=[];foreach($un
as$Q)$Gc[table($Q)]=view($Q);Connection::get()->selectDatabase($gm);$h=idf_escape(DB);foreach($Gc
as$A=>$sn){if(!queries("CREATE VIEW $A AS ".str_replace(" $h."," ",$sn["select"]))||!queries("DROP VIEW $h.$A"))return
false;}return
true;}return
false;}function
copy_tables(array$S,array$un,$gm){queries("SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO'");foreach($S
as$Q){$A=($gm==DB?table("copy_$Q"):idf_escape($gm).".".table($Q));if(($_POST["overwrite"]&&!queries("\nDROP TABLE IF EXISTS $A"))||!queries("CREATE TABLE $A LIKE ".table($Q))||!queries("INSERT INTO $A SELECT * FROM ".table($Q)))return
false;foreach(get_rows("SHOW TRIGGERS LIKE ".q(addcslashes($Q,"%_\\")))as$L){$Em=$L["Trigger"];if(!queries("CREATE TRIGGER ".($gm==DB?idf_escape("copy_$Em"):idf_escape($gm).".".idf_escape($Em))." $L[Timing] $L[Event] ON $A FOR EACH ROW\n$L[Statement];"))return
false;}}foreach($un
as$Q){$A=($gm==DB?table("copy_$Q"):idf_escape($gm).".".table($Q));$sn=view($Q);if(($_POST["overwrite"]&&!queries("DROP VIEW IF EXISTS $A"))||!queries("CREATE VIEW $A AS $sn[select]"))return
false;}return
true;}function
trigger($A,$Q){if($A=="")return[];$M=get_rows("SHOW TRIGGERS WHERE `Trigger` = ".q($A));return
reset($M);}function
triggers($Q){$K=[];foreach(get_rows("SHOW TRIGGERS LIKE ".q(addcslashes($Q,"%_\\")))as$L)$K[$L["Trigger"]]=[$L["Timing"],$L["Event"]];return$K;}function
trigger_options(){return["Timing"=>["BEFORE","AFTER"],"Event"=>["INSERT","UPDATE","DELETE"],"Type"=>["FOR EACH ROW"],];}function
routine($A,$U){if($A=="")return[];$M=get_rows("SELECT PARAMETER_NAME, DTD_IDENTIFIER, PARAMETER_MODE, CHARACTER_SET_NAME
FROM information_schema.PARAMETERS
WHERE SPECIFIC_SCHEMA = DATABASE() AND ROUTINE_TYPE = '$U' AND SPECIFIC_NAME = ".q($A)."
ORDER BY ORDINAL_POSITION");$l=[];foreach($M
as$L){$De=$L["DTD_IDENTIFIER"];list($Lm,$v,$Vm)=parse_type($De);$l[]=["field"=>$L["PARAMETER_NAME"],"type"=>$Lm,"length"=>$v,"unsigned"=>$Vm,"null"=>true,"full_type"=>$De,"inout"=>($U=="FUNCTION"?"":$L["PARAMETER_MODE"]),"collation"=>$L["CHARACTER_SET_NAME"],];}$K=Connection::get()->query("SELECT
	ROUTINE_COMMENT comment,
	CONCAT(IF(IS_DETERMINISTIC = 'YES', 'DETERMINISTIC\\n', ''), IF(SQL_DATA_ACCESS != 'CONTAINS SQL', CONCAT(SQL_DATA_ACCESS, '\\n'), '')) characteristics,
	ROUTINE_DEFINITION definition,
	'SQL' language
FROM information_schema.ROUTINES
WHERE ROUTINE_SCHEMA = DATABASE() AND ROUTINE_TYPE = '$U' AND ROUTINE_NAME = ".q($A))->fetchAssoc();$e=Connection::get()->getValue("SHOW CREATE $U ".idf_escape($A),2);$yl="'(?:[^'\\\\]|\\\\.)*'";$ym='`[^`]*`|"(?:[^"\\\]|\\\.)*"|$string|/\*.*?\*/|/';$qb="    (?:[A-Z ]+|COMMENT (?:$yl)+)\\n";if($e&&preg_match("~^(?:[^(`\"'/]++|$ym)*?(\\((?:[^()`\"'/]++|$ym|(?1))*+\\))(?: RETURNS [^\\n]*)?\\n(?:$qb)*~s",$e,$z))$K['definition']=substr($e,strlen($z[0]));$K['definition']=$K['characteristics'].$K['definition'];unset($K['characteristics']);if($l&&$l[0]['field']=='')$K['returns']=array_shift($l);$K['fields']=$l;return$K;}function
routines(){return
get_rows("SELECT SPECIFIC_NAME, ROUTINE_NAME, ROUTINE_TYPE, DTD_IDENTIFIER, ROUTINE_COMMENT FROM information_schema.ROUTINES WHERE ROUTINE_SCHEMA = DATABASE()");}function
routine_languages(){return[];}function
routine_id($A,array$L){return
idf_escape($A);}function
last_id($J){return
Connection::get()->getValue("SELECT LAST_INSERT_ID()");}function
explain(Connection$d,$I){return$d->query("EXPLAIN ".(Connection::get()->isMinVersion("5.7")?"":"PARTITIONS ").$I);}function
found_rows(array$R,array$Z){return$R["Engine"]=="InnoDB"&&!$Z?(int)$R["Rows"]:null;}function
format_sql($I){$Tg='(?:[^`\']|`[^`]*`|\'[^\']*\')*';$ng='FROM|WHERE|HAVING|GROUP\s+BY|ORDER\s+BY|(NATURAL\s+)?((LEFT|RIGHT)\s+)?((INNER|OUTER|CROSS)\s+)?JOIN';$I=preg_replace("~($Tg)\\s+(AS\\s+SELECT)~isU","$1 AS\nSELECT",$I);$I=preg_replace("~($Tg)\\s+($ng)~isU","$1\n$2",$I);$I=preg_replace("~($Tg),~isU","$1,\n  ",$I);return$I;}function
create_sql($Q,$Qa,$Cl){$I=Connection::get()->getValue("SHOW CREATE TABLE ".table($Q),1);if(!$Qa)$I=preg_replace('~ AUTO_INCREMENT=\d+~','',$I);return!str_contains($I,"\n")?format_sql($I):$I;}function
truncate_sql($Q){return"TRUNCATE ".table($Q);}function
create_database_sql($wc,$Cl=""){$A=idf_escape($wc);$Mb="";if(str_contains($Cl,"CREATE")&&($e=Connection::get()->getValue("SHOW CREATE DATABASE $A",1))){set_utf8mb4($e);if($Cl=="DROP+CREATE")$Mb="DROP DATABASE IF EXISTS $A;\n";$Mb
.="$e;\n";}return$Mb;}function
use_sql($wc,$Cl=""){return"USE ".idf_escape($wc).";\n";}function
trigger_sql($Q){$nl="";foreach(get_rows("SHOW TRIGGERS LIKE ".q(addcslashes($Q,"%_\\")),null,"-- ")as$L)$nl
.="\nCREATE TRIGGER ".idf_escape($L["Trigger"])." $L[Timing] $L[Event] ON ".table($L["Table"])." FOR EACH ROW\n$L[Statement];;\n";return$nl;}function
show_variables(){return
get_rows("SHOW VARIABLES");}function
show_status(){return
get_rows("SHOW STATUS");}function
process_list(){return
get_rows("SHOW FULL PROCESSLIST");}function
convert_field(array$k){if(preg_match("~binary~",$k["type"]))return"HEX(".idf_escape($k["field"]).")";if($k["type"]=="bit")return"BIN(".idf_escape($k["field"])." + 0)";if($k["type"]=="vector")return(Connection::get()->isMariaDB()?"VEC_ToText":"VECTOR_TO_STRING")."(".idf_escape($k["field"]).")";if(preg_match("~geometry|point|linestring|polygon~",$k["type"])){$yj=(Connection::get()->isMinVersion("8")?"ST_":"");$b=idf_escape($k["field"]);return"CONCAT(IF($yj"."SRID($b), CONCAT('SRID=', $yj"."SRID($b), ';'), ''), $yj"."AsWKT($b))";}return
null;}function
unconvert_field(array$k,$K){if(preg_match("~binary~",$k["type"]))$K="UNHEX($K)";if($k["type"]=="bit")$K="CONVERT(b$K, UNSIGNED)";if($k["type"]=="vector")$K=(Connection::get()->isMariaDB()?"VEC_FromText":"STRING_TO_VECTOR")."($K)";if(preg_match("~geometry|point|linestring|polygon~",$k["type"])){$rl=isset($k["srid"])?$k["srid"]:0;if(preg_match("~^'SRID=(\\d+);~i",$K,$z)){$rl=$z[1];$K="'".substr($K,strlen($z[0]));}$K=(Connection::get()->isMinVersion("8")?"ST_":"")."GeomFromText($K, $rl)";}return$K;}function
support($Yd){return
preg_match('~^(comment|columns|copy|database|drop_col|dump|event|indexes|kill|privileges|move_col|procedure|processlist|routine|sql|status|table|trigger|variables|view'.(Connection::get()->isMinVersion(Connection::get()->isMariaDB()?"10.8.1":"8")?'|descidx':'').(Connection::get()->isMinVersion(Connection::get()->isMariaDB()?"10.2.1":"8.0.16")?'|check':'').(!Connection::get()->isMariaDB()&&Connection::get()->isMinVersion("8")?'|fast_status':'').')$~',$Yd);}function
kill_process($X){return
queries("KILL ".number($X));}function
connection_id(){return"SELECT CONNECTION_ID()";}function
max_connections(){return(int)Connection::get()->getValue("SELECT @@max_connections");}}$qj="adminneo-plugins";if(is_dir($qj)){foreach(glob("$qj/*.php")as$n)include_once$n;}function
get_compressed_translations($vg){switch($vg){case'en':return'%X/&cbomt,|?z"**gW&<fbys!&IOEIP1geigv6#(#C3vCp^f]TGDYB[K"59W@k`x-Xw.Kn1lh0Q&t#Qb%IGnUgw$3u36/vM_*
<%SMQF=N<O$lz5jRynoNp6._7`CFqasK1jGxLF(Rg5%/KKgvic"&W=+qRf>jgB`bX]l_OiwUq/N#B=][3f!EYYl[MZwvPr,z)O*JDT
H+1A
mj*m^)jg/eK5i2L]]`GC!iUsR]]B+Qdn?MrBOfHG}jd5voufj_p2KMkscL$mYt@JUanvACnXk5a22(8S3r,&5ygUG2y$:GcQEWCKRDDFa43M
rvYZ6DYfHoAMk}fLwokHQ*E#CCsc(ID~u,3r;|Y8FqUe_lEX+TVU[;kcRwR?%mi;]pf0^H0N6*-$Uoe,Ag`I.[=}ZId,ohW|*gkRe1>ycXK3eUE$cTZJ
&5CD#?zG><P&Hkif:wWk!/2_c6MD&"0ED97ko%JS"KmLQqy>=@S$.@*,iR#p_F{$Fdp*jY
aAn04MhRt(vn[B4~w$#$v4cQdpyrs/9`s~7PK_^Ij.eg:UGjH|E/=Mol>V?Ap5[fE4iJq.L#Bg4}?48OaF<6H!3w?c/;`Zk7RqGp%tjX`2w`h(,Chm#*hCnxO^HL2tT46Ua5+DMs/>C@_WSF]Vku/PDz7rr%j1L;TDybG9kqt%do8zWi?X6O%nYn;V!KK:nJ<cfQA/sK0EP-u+y``to=:0R`3"jk?6I%r~lW>AXVbwO2jU1_;uB}+7Mql0A"Zrr2B@kigg*5W9Oa_7?yk3sGX:]G>dhVMTL<=8=FK_tK;Qe9!:41
/7tLlmUOC7$;YVZ-28LmD[<A?l6f321KLbN/OIkyyo/O+K#L,9?E2.Kxoler?uCRfr~x
d!77sd+BEBhsgXdYgjK~I?KA:py#0Vbxv++GM(ZLe]q31`FTVM4^#eROD^P@Bu@L-~q94CD>l`-`pL9/db[{LD$ZQ^e<KMZxn-?jHn%c="n1Zs7l_sIeiC^GmbHmhCeVQe*W6_83;LC@vcWKs&f9lsn$LJA60lKet~kEcJJ)FEeZ?(x=)q/cPt)y*)N-I@sZBO=_(<E`=>f@(za;XDisuq_2XsU
;py@7H>~QFhbuR"4^l?{)CeW:
K,E:aTR:D#kfrF57O4hf3//<v[t6z)o
EjxXqfyzs`f=x98wR{^VLf#J;6VCvaa,5(($Jr2]NR!npClH,_xgr|nG`j*5]6SF@,k:su]W*D!-aoY*v1Ha<81hiu
,Ceh|)Z3E3vX+("z)qZnsT+*RFp!0NVy7
]A0)G#Y+O7DOd!QY16k@|koQL8&-;VJb_!UvS`,FSryKQfd;49G.sol[^iHPq]jePWhl%+`up(@$ovd=L;W5|"l;/feif-KE6,l%JrPP!2ZD|MA4[Vm23ZS;-E5cQ0Bt!0oo(dOQ:``i_5b3*$.YDAx[=]fOW_}r)$p)^Q`vzR:5rlb(5r^./E"KbVBH0^6:WX@uiHo2A&;;21!/MJnN}1*%DL40T:SA{6e7puA@hx>=%J|a420h%)-y+F2e*/qU<gC86kgCF@Q:yYrG0dOd;X"c9BB=cpCLT:$5gA:@*bq#Y+G`RA+.S/yx6#AYa=DTk
FA2-ODk)@">#,XXdN
S]mNFp%/;&KF@b]aryauYB*ZdGAV~,XK~:uW]wbK)HVm3H{+31WBGqOn<U5PrIdT+#+yZ^EZXp<UA%vuljxw"[
$UpMozyD:Mx12&bAFiCW)cSBDbA}i4vWN.fbGTHhG4#-TF(|BDo^F(]*4b@fWJDlQZC8MX=cAk"^*1W"@db7SMe{G!](k
TRiL/6j+JEt|)s`KiUOof~8yd9+^it0a;&^prp[rrW0$vEJdR&Qd+&,&/.u`s";18!d)oHBVdVurDB,!O-^XH:Kw@Enf2b,/?<-C9Xg5MK^3-xtgcOkRjk#Mco8g]/.qqNV^b8v0dG0
NwgX/y#QemuW.yPpaf1F"mtZ)M`AqNOyyzPPotw/g{^.^80wbHaS;tOQjI_aC0#;-C.xpVvT[78iX"vVlgg=xUI#^ucaw56f3FC^-ylWR:_BR(4:O-gkWQI)J>?WF4x/Or$sYO^"]MmwZ7#Jv.s
(r`;2H3qf<`G3LM0==9HF<a8BZYB"&2tZ:p8&/bghHS)>/iL7=8,/{
k0DcBL51xrXacQ
?4".1-5z><A@DrBTUvk>q->>%)tnG7q^kr?G4c_H8WF|!87`Wg@xo$=R)*2u<G*~6P=?["yRX)aHX5D(/:t*v>(Rs%$82*yINID+h},g$zHqaMy~(dt[
fnT7%bqg(GM]Lf}]
3DFeY<*=G{7#3}Tr*I$K($q1+pr4i7/U!eeKG5op(>V+DMWzisS8J!qvT$XW,TWs!p03/;koZ,QG8eb_45UE=R6Qt96eiwR>V|<JPPydv/Cv-Wgh1]Y@c;5{$ys83~!|=6Id!L6bv-:MY<oiX_cIrJ!oIIa9>><qfn0bcoO8z"xOw8ST;O&yhdOGS{(wv77U(L>6:3Se]6R}1&3f41_?/
]S(>vn+y?$T5vn8+TmA.^4>$S>q$VW/;gKu|[uS2iaRRJ!#r70@j$<G|7kTMBB-DWht?r@CY?rJ{-du-3Qd;%G70*:KooP5YDV?rI`kgA(VeIH5M[kWTt^dL-S.q
P9nP_+kV_24A?Ip={S^X^#bhSL[w:u@yP9)V[.mFyKy#jD~1x4g<RmX-.:l@*IhPX
tS;I-
>q"3&sqwd%TmWrG#{=BKVPRAZM!3
@umT6MHwLw2n/B1{5JYufP_Ue7U(:2%~?|Jf+;]V5s^(s$&:faGDTr0/Jzr-1p7?frc$iL(JNI%@uy^V<$kzg5Q]T]CflA6v`7:thavI+>0S#aA:f4pqw|:$mO,KdN04-#9fGvtSsKX,<E[MvYDt-gfT]Af&wF)?e,,uMb=sOpixV;Txla+x*lh|K$K/TA7g-WwHaQTa.-:)TmZ^/v[SPinyacQK6%O6EO;9eLug]:aP%Zu;?qv8:f0Y#9J`<P>[E[=b>[A^EBf}GXt}5lltvhNq7DK_v2:31*cw?2V|o{WMf<-<Oa?/k2=n+rphh:<th0!b?5BYg{)*OgWvx;4gb?BnmBd#AR<cqzqQJ0MxE[>35g:5xA?UN>/cLYEZ9"pti<IIDqr&#u5o6hvMWOs@,dUp(UuI;8fm^N';}}function
get_translations($vg){$Nc=$vg!="en"?decompress_string(get_compressed_translations("en")):"";return
json_decode(decompress_string(get_compressed_translations($vg),$Nc),true);}function
get_plural_translation_id($u){$rj=array('Too many unsuccessful logins, try again in %d minute(s).'=>142,'%d process(es) have been killed.'=>281,'%d query(s) executed OK.'=>199,'Query executed OK, %d row(s) affected.'=>197,'%d row(s) have been imported.'=>288,'Routine has been called, %d row(s) affected.'=>233,'%d row(s)'=>196,'%d byte(s)'=>43,'%d item(s) have been affected.'=>285,);return
isset($rj[$u])?$rj[$u]:null;}$Dm=$_SESSION["translations"];$wg=Locale::get()->getLanguage();if($_SESSION["translations_version"]!=1904488331){$Dm=[];$_SESSION["translations_version"]=1904488331;}if($_SESSION["translations_language"]!=$wg){$Dm=[];$_SESSION["translations_language"]=$wg;}if(!$Dm){$Dm=get_translations($wg);$_SESSION["translations"]=$Dm;}Locale::get()->setTranslations($Dm);$ya=null;$qc=false;$Jf=null;if(function_exists('\adminneo_instance')){$ya=\adminneo_instance();$qc=true;}elseif(file_exists("adminneo-instance.php")){$ya=include_once"adminneo-instance.php";$qc=true;}if($qc&&!$ya
instanceof
Admin&&!$ya
instanceof
Pluginer){$ya=null;$Lg="href=https://github.com/adminneo-org/adminneo#advanced-customizations ".target_blank();$Jf=lang(131,"<b>adminneo-instance.php</b>","<b>adminneo_instance()</b>","Admin::create()")." <a $Lg>".lang(1)."</a>";}if(!$ya)$ya=Admin::create();if($Jf)$ya->addError($Jf);if($xj!==null&&!isset($_GET["settings"])){$ya->getSettings()->updateParameter("lang",$xj);redirect(remove_from_uri());}if(!defined("AdminNeo\DRIVER")){define("AdminNeo\DRIVER",null);define("AdminNeo\DIALECT",null);}if(DRIVER&&!is_string($_GET[DRIVER])){http_response_code(400);exit;}define("AdminNeo\SERVER",DRIVER?$_GET[DRIVER]:null);define("AdminNeo\DB",isset($_GET["db"])?$_GET["db"]:"");define("AdminNeo\BASE_URL",preg_replace('~\?.*~','',relative_uri()));define("AdminNeo\ME",BASE_URL.'?'.(sid()?session_name()."=".urlencode(session_id()).'&':'').($_GET["ext"]?"ext=".urlencode($_GET["ext"]).'&':'').(SERVER!==null?DRIVER."=".urlencode(SERVER).'&':'').(isset($_GET["username"])?"username=".urlencode($_GET["username"]).'&':'').(DB!=""?'db='.urlencode(DB).'&'.(isset($_GET["ns"])?"ns=".urlencode($_GET["ns"])."&":""):''));define("AdminNeo\HOME_URL",BASE_URL?:".");define("AdminNeo\SERVER_HOME_URL",substr(preg_replace('~\b(username|db|ns)=[^&]*&~','',ME),0,-1)?:".");if(isset($_GET["set"])){header("Content-Type: text/javascript; charset=utf-8");if(!verify_token()){header("HTTP/1.1 403 Forbidden");exit;}if($_GET["set"]=="navigation-width"){$Hn=isset($_POST["width"])?$_POST["width"]:"";if($Hn!=""){$Hn=min(max((float)$Hn,Settings::$NavigationWidthMin),Settings::$NavigationWidthMax);Admin::get()->getSettings()->updateParameter("navigationWidth",sprintf("%.2F",$Hn));}else
Admin::get()->getSettings()->updateParameter("navigationWidth",null);}if($_GET["set"]=="export-settings")Admin::get()->getSettings()->updateParameters(["exportFormat"=>isset($_POST["format"])?$_POST["format"]:"","exportOutput"=>isset($_POST["output"])?$_POST["output"]:"",]);exit;}const
VERSION="5.9.1";function
page_header($T,$fb=[]){if(!headers_sent()&&!array_sum(array_column(ob_get_status(true),"buffer_used")))ini_set("zlib.output_compression","1");page_headers();if(is_ajax()&&Admin::get()->getErrors()){page_messages();exit;}if(!ob_get_level())ob_start(null,4096);$T=strip_tags($T);$Uk=$fb!==false&&$fb!==null&&SERVER!=""?" - ".h(Admin::get()->getServerName(SERVER)):"";$Wk=strip_tags(Admin::get()->getServiceTitle());$um=$T.$Uk." - ".($Wk!=""?$Wk:"AdminNeo");echo"<!DOCTYPE html>\n","<html lang='",Locale::get()->getLanguage(),"' dir='",lang(132),"' class='",lang(132)," nojs'>\n","<head>\n","<meta http-equiv='Content-Type' content='text/html; charset=utf-8'>\n","<meta name='robots' content='noindex, nofollow'>\n","<meta name='viewport' content='width=device-width, initial-scale=1'>\n","<title>$um</title>\n";$Hb=validate_color_variant(Admin::get()->getConfig()->getColorVariant());echo"<link rel='stylesheet' href='",h(link_files("default-$Hb.css",[])),"'>\n";if(!Admin::get()->isLightModeForced())echo"<link rel='stylesheet' ".(!Admin::get()->isDarkModeForced()?"media='(prefers-color-scheme: dark)' ":"")."href='",h(link_files("default-$Hb-dark.css",[])),"'>\n";$nm=Admin::get()->getSettings()->getTheme();list($nm,$Hb)=validate_theme($nm,$Hb);if($nm!="default"){echo"<link rel='stylesheet' href='",h(link_files("$nm-$Hb.css",[])),"'>\n";if(!Admin::get()->isLightModeForced())echo"<link rel='stylesheet' ".(!Admin::get()->isDarkModeForced()?"media='(prefers-color-scheme: dark)' ":"")."href='",h(link_files("$nm-$Hb-dark.css",[])),"'>\n";}foreach(Admin::get()->getCssUrls()as$an){if(strpos($an,"adminneo-dark.css")===0&&!Admin::get()->isDarkModeForced())echo"<link rel='stylesheet' media='(prefers-color-scheme: dark)' href='",h($an),"'>\n";else
echo"<link rel='stylesheet' href='",h($an),"'>\n";}$Mh=Admin::get()->getSettings()->getNavigationWidth();echo"<style id='navigation-width'>";if($Mh)echo"@media screen and (min-width: 1024px) { :root { --menu-width: ",sprintf("%.2F",$Mh),"rem } }";echo"</style>\n",script_src(link_files("main.js",[]));foreach(Admin::get()->getJsUrls()as$an)echo
script_src($an);Admin::get()->printFavicons();Admin::get()->printToHead();echo"</head>\n","<body>\n",script("const offlineMessage = '".js_escape(lang(133))."';\n"."const thousandsSeparator = '".js_escape(lang(108))."';"),"<div id='help' class='jush-".DIALECT." jsonly hidden'></div>",script("initHelpPopup();"),'<menu class="access-menu">','<li><a href="#main-content">'.lang(134).'</a></li>','<li><a class="panel-link" href="#navigation-panel">'.lang(135).'</a></li>';if($fb!==null&&DB!=""&&$_GET["ns"]!=="")echo'<li><a class="panel-link" href="#tables">'.lang(136).'</a></li>';echo'</menu>',"<div id='content'>\n","<div class='header'>\n","<button id='open-navigation-button' type='button' class='button light navigation-button' title='",lang(137),"' aria-controls='navigation-panel' aria-expanded='false'>",icon_solo("menu"),"</button>";if($fb!==null){echo'<nav class="breadcrumbs"><ul>',"<li><a href='".h(HOME_URL)."' title='",lang(138),"'>",icon_solo("home"),"</a></li>";$Sk=h(Admin::get()->getServerName((string)SERVER));if($fb===false)echo"<li>$Sk</li>";else{$x=substr(preg_replace('~\b(db|ns)=[^&]*&~','',ME),0,-1);echo"<li><a href='".h($x)."' accesskey='1' title='Alt+Shift+1'>$Sk</a></li>";if($_GET["ns"]!=""||(DB!=""&&is_array($fb)))echo'<li><a href="'.h($x."&db=".urlencode(DB).(support("scheme")?"&ns=":"")).'">'.h(DB).'</a></li>';if($fb===true){if($_GET["ns"]!="")echo'<li>'.h($_GET["ns"]).'</li>';else
echo"<li>",h(DB),"</li>";}else{if($_GET["ns"]!="")echo'<li><a href="'.h(substr(ME,0,-1)).'">'.h($_GET["ns"]).'</a></li>';foreach($fb
as$u=>$X){if(is_string($u)){$Kc=(is_array($X)?$X[1]:h($X));if($Kc!="")echo"<li><a href='".h(ME."$u=").urlencode(is_array($X)?$X[0]:$X)."'>$Kc</a></li>";}else
echo"<li>$X</li>\n";}}}echo"</ul></nav>";}echo"</div>\n","<div id='main-content'>\n","<h1>$T</h1>\n","<div id='ajaxstatus' role='status' class='jsonly'></div>\n";restart_session();page_messages();$g=&get_session("dbs");if(DB!=""&&$g&&!in_array(DB,$g,true))$g=null;stop_session();define("AdminNeo\PAGE_HEADER",1);ob_flush();flush();}function
validate_color_variant($Hb){list(,$Hb)=validate_theme("default",$Hb);return$Hb;}function
validate_theme($nm,$Hb){$om=get_available_themes();if(isset($om[$nm][$Hb]))return[$nm,$Hb];if(isset($om["default"][$Hb]))return["default",$Hb];reset($om["default"]);return["default",key($om["default"])];}function
get_available_themes(){return
array('default'=>array('blue'=>true,),);}function
get_theme_titles($Hb){$om=get_available_themes();$vm=[];foreach($om
as$nm=>$Ib){if(isset($Ib[$Hb])?$Ib[$Hb]:false)$vm[$nm]=($nm=="default"?"Neo":ucwords(str_replace("-"," ",$nm)));}return$vm;}function
page_headers(){header("Content-Type: text/html; charset=utf-8");header("Cache-Control: no-cache");header("X-XSS-Protection: 0");header("X-Content-Type-Options: nosniff");header("Referrer-Policy: origin-when-cross-origin");header("X-Frame-Options: DENY");$nc=["script-src"=>"'self' 'unsafe-inline' 'nonce-".get_nonce()."' 'strict-dynamic'","connect-src"=>"'self' https://api.github.com/repos/adminneo-org/adminneo/releases/latest","frame-src"=>"'self'","object-src"=>"'none'","base-uri"=>"'none'","form-action"=>"'self'",];Admin::get()->updateCspHeader($nc);$Qc=[];foreach($nc
as$Pc=>$ll)$Qc[]="$Pc $ll";header("Content-Security-Policy: ".implode("; ",$Qc));Admin::get()->sendHeaders();}function
get_nonce(){static$Wh;if(!$Wh)$Wh=Random::strongKey();return$Wh;}function
page_messages(){$Zm=preg_replace('~^[^?]*~','',$_SERVER["REQUEST_URI"]);$rh=isset($_SESSION["messages"][$Zm])?$_SESSION["messages"][$Zm]:null;if($rh){foreach($rh
as$nh)echo"<div class='message'>$nh</div>\n",script("initToggles(qsl('.message'));");unset($_SESSION["messages"][$Zm]);}foreach(Admin::get()->getErrors()as$j)echo"<div class='error'>$j</div>\n";}function
page_footer($xh=null){echo"</div>\n","</div>\n","<div id='navigation-panel' class='navigation-panel'>\n","<div class='focus-trap-begin'></div>\n";$zg=isset($_COOKIE["neo_version"])?$_COOKIE["neo_version"]:null;echo"<div class='header'>\n","<button id='close-navigation-button' type='button' class='button light navigation-button' title='",lang(139),"' aria-controls='navigation-panel'>",icon_solo("close"),"</button>",Admin::get()->getServiceTitle()."\n";if($xh!="auth"){echo"<span class='version'>",h(preg_replace('~\\.0(-|$)~','$1',VERSION));if(Admin::get()->getConfig()->isVersionVerificationEnabled()&&$zg&&version_compare(VERSION,$zg)<0)echo"<a id='version' class='version-badge' href='https://www.adminneo.org/download' ".target_blank()." title='".h($zg)."'>",icon_solo("asterisk"),"</a>";echo"</span>\n";if(Admin::get()->getConfig()->isVersionVerificationEnabled()&&!$zg)echo
script("verifyVersion();");}echo"</div>\n";Admin::get()->printNavigation($xh);echo"<div class='footer'>\n","<div class='toolbox'>";if($xh=="auth")language_select();else{$x=h(preg_replace('~\b(db|ns)=[^&]*&~',"",ME)."settings=");echo"<a class='button light' title='",lang(140),"' href='$x'>",icon_solo("settings"),"</a>";}echo"</div>";if($xh!="auth")Admin::get()->printLogout();echo"</div>\n","<div id='navigation-resizer' class='navigation-resizer'></div>\n","<div class='focus-trap-end'></div>\n","</div>\n",script("initNavigation(); initNavigationResizer('".js_escape(ME)."set=navigation-width', '".get_token()."', ".Settings::$NavigationWidthMin.", ".Settings::$NavigationWidthMax.");");}function
int32($Hh){while($Hh>=2147483648)$Hh-=4294967296;while($Hh<=-2147483649)$Hh+=4294967296;return(int)$Hh;}function
long2str(array$W,$zn){$sk='';foreach($W
as$X)$sk
.=pack('V',$X);return$zn?substr($sk,0,end($W)):$sk;}function
str2long($sk,$zn){$W=array_values(unpack('V*',str_pad($sk,4*ceil(strlen($sk)/4),"\0")));if($zn)$W[]=strlen($sk);return$W;}function
xxtea_mx($Ln,$Kn,$Gl,$hg){return
int32((($Ln>>5&0x7FFFFFF)^$Kn<<2)+(($Kn>>3&0x1FFFFFFF)^$Ln<<4))^int32(($Gl^$Kn)+($hg^$Ln));}function
xxtea_encrypt_string($nj,$u){$u=array_values(unpack("V*",pack("H*",md5($u))));$W=str2long($nj,true);$Hh=count($W)-1;$Ln=$W[$Hh];$Kn=$W[0];$Jj=floor(6+52/($Hh+1));$Gl=0;while($Jj-->0){$Gl=int32($Gl+0x9E3779B9);$md=$Gl>>2&3;for($Qi=0;$Qi<$Hh;$Qi++){$Kn=$W[$Qi+1];$Fh=xxtea_mx($Ln,$Kn,$Gl,$u[$Qi&3^$md]);$Ln=int32($W[$Qi]+$Fh);$W[$Qi]=$Ln;}$Kn=$W[0];$Fh=xxtea_mx($Ln,$Kn,$Gl,$u[$Qi&3^$md]);$Ln=int32($W[$Hh]+$Fh);$W[$Hh]=$Ln;}return
long2str($W,false);}function
xxtea_decrypt_string($f,$u){$u=array_values(unpack("V*",pack("H*",md5($u))));$W=str2long($f,false);$Hh=count($W)-1;$Ln=$W[$Hh];$Kn=$W[0];$Jj=floor(6+52/($Hh+1));$Gl=int32($Jj*0x9E3779B9);while($Gl){$md=$Gl>>2&3;for($Qi=$Hh;$Qi>0;$Qi--){$Ln=$W[$Qi-1];$Fh=xxtea_mx($Ln,$Kn,$Gl,$u[$Qi&3^$md]);$Kn=int32($W[$Qi]-$Fh);$W[$Qi]=$Kn;}$Ln=$W[$Hh];$Fh=xxtea_mx($Ln,$Kn,$Gl,$u[$Qi&3^$md]);$Kn=int32($W[0]-$Fh);$W[0]=$Kn;$Gl=int32($Gl-0x9E3779B9);}return
long2str($W,true);}const
ENCRYPTION_GCM='aes-256-gcm';const
ENCRYPTION_CBC='aes-256-cbc';const
ENCRYPTION_TAG_LENGTH=16;const
ENCRYPTION_HMAC_LENGTH=64;function
generate_iv($v){if(function_exists('random_bytes')){try{return
random_bytes($v);}catch(Exception$md){}}return
openssl_random_pseudo_bytes($v);}function
hash_key($u){return
substr(hash('sha512',$u,true),0,32);}function
aes_encrypt_string($nj,$u){$vh=PHP_VERSION_ID>=70100&&in_array(ENCRYPTION_GCM,openssl_get_cipher_methods())?ENCRYPTION_GCM:ENCRYPTION_CBC;$u=hash_key($u);$cg=generate_iv(openssl_cipher_iv_length($vh)?:16);if($vh==ENCRYPTION_GCM)$_b=openssl_encrypt($nj,$vh,$u,OPENSSL_RAW_DATA,$cg,$em,"",ENCRYPTION_TAG_LENGTH);else{$_b=openssl_encrypt($nj,$vh,$u,OPENSSL_RAW_DATA,$cg);$em=hash_hmac("sha512",$cg.$_b,$u,true);}if($_b===false)return
false;return$cg.$em.$_b;}function
aes_decrypt_string($f,$u){$vh=PHP_VERSION_ID>=70100&&in_array(ENCRYPTION_GCM,openssl_get_cipher_methods())?ENCRYPTION_GCM:ENCRYPTION_CBC;$dg=openssl_cipher_iv_length($vh)?:16;$fm=$vh==ENCRYPTION_GCM?ENCRYPTION_TAG_LENGTH:ENCRYPTION_HMAC_LENGTH;if(strlen($f)<$dg+$fm)return
false;$u=hash_key($u);$cg=substr($f,0,$dg);$em=substr($f,$dg,$fm);$_b=substr($f,$dg+$fm);if($cg===false||$em===false||$_b===false)return
false;if($vh==ENCRYPTION_GCM)return
openssl_decrypt($_b,$vh,$u,OPENSSL_RAW_DATA,$cg,$em);else{$hf=hash_hmac('sha512',$cg.$_b,$u,true);if(!hash_equals($em,$hf))return
false;return
openssl_decrypt($_b,$vh,$u,OPENSSL_RAW_DATA,$cg);}}function
encrypt_string($nj,$u){if($nj=="")return"";if(extension_loaded('openssl'))return
aes_encrypt_string($nj,$u);else
return
xxtea_encrypt_string($nj,$u);}function
decrypt_string($f,$u){if($f=="")return"";if(extension_loaded('openssl'))return
aes_decrypt_string($f,$u);else
return
xxtea_decrypt_string($f,$u);}$G=[];if($_COOKIE["neo_permanent"]){foreach(explode(" ",$_COOKIE["neo_permanent"])as$X){list($u)=explode(":",$X);$G[$u]=$X;}}function
validate_server_input(array&$G){if(Admin::get()->getConfig()->getServer(SERVER))return;$O=preg_replace('~:/[-\w.][-\w.:/]*$~D',"",SERVER);if($O=="")return;if(preg_match('~[[:cntrl:][:space:]]~',$O))auth_error($G);if(!preg_match('~^[^:]+://~',$O))$O="https://$O";$ej=parse_url($O);if(!$ej)auth_error($G);if(isset($ej['user'])||isset($ej['pass'])||isset($ej['query'])||isset($ej['fragment']))auth_error($G);if(isset($ej['scheme'])&&!preg_match('~^(https?)$~i',$ej['scheme']))auth_error($G);$kf=$ej['host'].(isset($ej['path'])?$ej['path']:'');if(!is_server_host_valid($kf))auth_error($G);if(isset($ej['port'])&&($ej['port']<1024||$ej['port']>65535))auth_error($G,lang(141));}function
validate_credentials_input(array&$G){$F=get_password();if(strpos($_GET["username"],"\0")!==false||(is_string($F)&&strpos($F,"\0")!==false))auth_error($G);}if(!function_exists('AdminNeo\is_server_host_valid')){function
is_server_host_valid($kf){return
is_host_valid($kf);}}function
build_http_url($O,$V,$F,$Dc,$Cc=null){if(!preg_match('~^(https?://)?([^:]*)(:\d+)?$~',rtrim($O,'/'),$_))return
null;return($_[1]?:"http://").($V!==""||$F!==""?urlencode($V).":".urlencode($F)."@":"").($_[2]!==""?$_[2]:$Dc).(isset($_[3])?$_[3]:($Cc?":$Cc":""));}function
add_invalid_login(){$Za=get_temp_dir()."/adminneo-invalid";$m=null;foreach(glob("$Za*")?:[$Za]as$n){$m=open_file_with_lock($n);if($m)break;}if(!$m){$m=open_file_with_lock("$Za-".Random::strongKey());if(!$m)return;}$Of=json_decode(stream_get_contents($m),true);$qm=time();if($Of){foreach($Of
as$Pf=>$X){if($X[0]<$qm)unset($Of[$Pf]);}}$Nf=&$Of[Admin::get()->getBruteForceKey()];if(!$Nf)$Nf=[$qm+30*60,0];$Nf[1]++;write_and_unlock_file($m,json_encode($Of));}function
check_invalid_login(array&$G){$Za=get_temp_dir()."/adminneo-invalid";$Of=[];foreach(glob("$Za*")as$n){$m=open_file_with_lock($n);if($m){$Of=json_decode(stream_get_contents($m),true);unlock_file($m);break;}}$Nf=($Of?$Of[Admin::get()->getBruteForceKey()]:[]);$Uh=($Nf&&$Nf[1]>29?$Nf[0]-time():0);if($Uh>0)auth_error($G,lang(142,ceil($Uh/60)));}function
connect_to_db(array&$G){if(Admin::get()->getConfig()->hasServers()&&!Admin::get()->getConfig()->getServer(SERVER))auth_error($G);$d=connect(true,$j);if(!$d)connection_error(nl2br(h($j)),$G);return$d;}function
authenticate(array&$G){$J=Admin::get()->authenticate($_GET["username"],get_password());if($J!==true)connection_error($J,$G);}function
connection_error($j,array&$G){$j=$j?:lang(3);if(preg_match('~^ +| +$~',get_password()))$j
.="<br>".lang(143);auth_error($G,$j);}Admin::get()->init();$Oa=isset($_POST["auth"])?$_POST["auth"]:null;if($Oa){session_regenerate_id();$O=isset($Oa["server"])?$Oa["server"]:"";$Tk=Admin::get()->getConfig()->getServer($O);$cd=$Tk?$Tk->getDriver():(isset($Oa["driver"])?$Oa["driver"]:"");$O=$Tk?$O:trim($O);$V=isset($Oa["username"])?$Oa["username"]:"";$F=isset($Oa["password"])?$Oa["password"]:"";if($Tk&&$Tk->hasCredentials()&&$V==""&&$F==""){$V=$Tk->getUsername();$F=$Tk->getPassword();}$h=$Tk?$Tk->getDatabase():(isset($Oa["db"])?$Oa["db"]:"");save_login($cd,$O,$V,$F,$h);unset($_SESSION["permanent"][$cd][$O][$V]);$_SESSION["permanent_request"]=$Oa["permanent"]?permanent_key($cd,$O,$V,$h):null;if(count($_POST)==1||DRIVER!=$cd||SERVER!=$O||$_GET["username"]!==$V||DB!=$h)redirect(auth_url($cd,$O,$V,$h));}elseif($_POST["logout"]&&(!$_SESSION["token"]||verify_token())){foreach(["pwds","db","dbs","queries","permanent"]as$u)set_session($u,null);unset_permanent($G);redirect(SERVER_HOME_URL,lang(144));}elseif($G&&!$_SESSION["pwds"]){session_regenerate_id();$Cj=Admin::get()->getPrivateKey();foreach($G
as$u=>$X){list(,$zb)=explode(":",$X);list($cd,$O,$V,$h)=array_map("base64_decode",explode("-",$u));$F=$Cj?decrypt_permanent(base64_decode($zb),$Cj):false;save_login($cd,$O,$V,$F,$h);if($F!==false)$_SESSION["permanent"][$cd][$O][$V]=true;}}function
permanent_key($cd,$O,$V,$h){return
implode("-",array_map("base64_encode",[$cd,$O,$V,$h]));}function
encrypt_permanent($F,$Cj){return
encrypt_string((time()+2592000).":$F",$Cj);}function
decrypt_permanent($f,$Cj){$nj=decrypt_string($f,$Cj);if($nj===false||!preg_match('~^(\d+):(.*)$~Ds',$nj,$_)||$_[1]<time())return
false;return$_[2];}function
issue_permanent(array&$G){$u=isset($_SESSION["permanent_request"])?$_SESSION["permanent_request"]:"";if(!$u||strpos($u,permanent_key(DRIVER,SERVER,$_GET["username"],""))!==0)return;unset($_SESSION["permanent_request"]);$Cj=Admin::get()->getPrivateKey(true);$xd=$Cj?encrypt_permanent(get_password(),$Cj):false;$G[$u]="$u:".base64_encode($xd?:"");cookie("neo_permanent",implode(" ",$G));}function
unset_permanent(array&$G){foreach($G
as$u=>$X){list($cd,$O,$V,$h)=array_map("base64_decode",explode("-",$u));if($cd==DRIVER&&$O==SERVER&&$V==$_GET["username"]&&$h==DB)unset($G[$u]);}cookie("neo_permanent",implode(" ",$G));}function
auth_error(array&$G,$j=null){$Xk=session_name();if(isset($_GET["username"])){header("HTTP/1.1 403 Forbidden");if(($_COOKIE[$Xk]||$_GET[$Xk])&&!$_SESSION["token"])$j=lang(145);else{restart_session();add_invalid_login();$F=get_password();if($F!==null){if($F===false)$j=lang(146);delete_login(DRIVER,SERVER,$_GET["username"]);}unset_permanent($G);}}if(!$_COOKIE[$Xk]&&$_GET[$Xk]&&ini_bool("session.use_only_cookies"))$j=lang(147);if(!$j)$j=lang(3);Admin::get()->addError($j);print_login_page();}function
print_login_page(){$Ti=session_get_cookie_params();cookie("neo_key",($_COOKIE["neo_key"]?:Random::strongKey()),$Ti["lifetime"]);if(!$_SESSION["token"])$_SESSION["token"]=rand(1,1e6);page_header(lang(32),null);echo"<form action='' method='post'>\n","<div>";if(print_hidden_fields($_POST,["auth"]))echo"<p class='message'>".lang(148)."\n";echo"</div>\n";Admin::get()->printLoginForm();echo"</form>\n";page_footer("auth");exit;}if(isset($_GET["username"])&&!DRIVER)print_login_page();if(isset($_GET["username"])&&!defined('AdminNeo\DRIVER_EXTENSION')){Admin::get()->addError(lang(149,implode(", ",Drivers::getExtensions(DRIVER))));unset($_SESSION["pwds"][DRIVER]);unset_permanent($G);page_header(lang(150),false);page_footer("auth");exit;}if(!isset($_GET["username"])||get_password()===null)print_login_page();validate_server_input($G);validate_credentials_input($G);check_invalid_login($G);Admin::get()->getConfig()->applyServer(SERVER);$d=connect_to_db($G);authenticate($G);issue_permanent($G);create_driver($d);if($_POST["logout"]&&$_SESSION["token"]&&!verify_token()){Admin::get()->addError(lang(151));page_header(lang(7));page_footer("db");exit;}if(!$_SESSION["token"])$_SESSION["token"]=rand(1,1e6);stop_session(true);if($Oa&&$_POST["token"])$_POST["token"]=get_token();if($_POST){if(!verify_token()){Admin::get()->addError(lang(151).' '.lang(152));$_POST=[];}}elseif($_SERVER["REQUEST_METHOD"]=="POST"){$j=lang(153,"'post_max_size'");if(isset($_GET["sql"]))$j
.=' '.lang(154);Admin::get()->addError($j);}is_authenticated(true);if(isset($_GET["settings"])){$P=Admin::get()->getSettings();$al=array_merge(Admin::get()->getSettingsRows(1),Admin::get()->getSettingsRows(2),Admin::get()->getSettingsRows(3));if($_POST){$Ti=[];foreach($al
as$u=>$L){if(isset($_POST[$u])){$cn=$_POST[$u]===""||(is_array($_POST[$u])&&in_array("",$_POST[$u]));$Ti[$u]=(!$cn?$_POST[$u]:null);}}$P->updateParameters($Ti);redirect(remove_from_uri());}$T=lang(140);page_header($T,[$T]);echo"<form id='settings' action='' method='post'>\n","<table class='box'>\n";foreach($al
as$L)echo$L;echo"</table>\n","<p>","<input type='submit' value='".lang(116),"' class='button default hidden'>",input_token(),"</p>\n","</form>\n",script("initSettingsForm();");page_footer();exit;}if(isset($_GET["status"]))$_GET["variables"]=$_GET["status"];if(isset($_GET["import"]))$_GET["sql"]=$_GET["import"];if(DB==""&&isset($_GET["ns"]))redirect(remove_from_uri('ns'));if(!(DB!=""?Connection::get()->selectDatabase(DB):(isset($_GET["sql"])||isset($_GET["dump"])||isset($_GET["database"])||isset($_GET["processlist"])||isset($_GET["privileges"])||isset($_GET["user"])||isset($_GET["variables"])||$_GET["script"]=="connect"||$_GET["script"]=="kill"))){if(DB!=""||$_GET["refresh"]){restart_session();set_session("dbs",null);}if(DB!=""){Admin::get()->addError(lang(155));header("HTTP/1.1 404 Not Found");page_header(lang(31).": ".h(DB),true);}else{if($_POST["db"])queries_redirect(substr(ME,0,-1),lang(156),drop_databases($_POST["db"]));$T=h(Drivers::get(DRIVER).": ".Admin::get()->getServerName(SERVER));page_header($T,false);$Mg=['privileges'=>[lang(73),"users"],'processlist'=>[lang(157),"list"],'variables'=>[lang(158),"variable"],'status'=>[lang(159),"status"],];$Ng="";foreach($Mg
as$u=>$X){if(support($u))$Ng
.="<a href='".h(ME)."$u='>".icon($X[1])."$X[0]</a>";}if($Ng)echo"<p class='links top-links'>$Ng</p>\n";echo"<p>".lang(160,Drivers::get(DRIVER),"<b>".h(Connection::get()->getVersion())."</b>","<b>".DRIVER_EXTENSION."</b>")."\n","<p>".lang(161,"<b>".h(logged_user())."</b>")."\n";$g=Admin::get()->getDatabases();if($g){$Bk=support("scheme");$Ea=collations();echo"<form action='' method='post'>\n","<div class='table-footer-parent'>\n","<div class='scrollable'>\n","<table class='checkable'>\n","<thead><tr>".(support("database")?"<th>":"")."<th aria-sort='ascending'>".lang(31).(get_session("dbs")!==null?" - <a href='".h(ME)."refresh=1'>".lang(162)."</a>":"")."<td>".lang(47)."<td>".lang(163)."<td>".lang(164)." - <a href='".h(ME)."dbsize=1'>".lang(165)."</a>".script("qsl('a').onclick = partial(ajaxSetHtml, '".js_escape(ME)."script=connect');","")."</thead>\n","<tbody>\n";$g=($_GET["dbsize"]?count_tables($g):array_flip($g));foreach($g
as$h=>$S){$nk=h(ME)."db=".urlencode($h);$r=h("Db-".$h);echo"<tr>".(support("database")?"<th class='actions'>".checkbox("db[]",$h,in_array($h,(array)$_POST["db"]),"","","",$r):""),"<th><a href='$nk' id='$r'>".h($h)."</a>";$Eb=h(db_collation($h,$Ea));echo"<td>".(support("database")?"<a href='$nk".($Bk?"&amp;ns=":"")."&amp;database=' title='".lang(70)."'>$Eb</a>":$Eb),"<td align='right'><a href='$nk&amp;schema=' id='tables-".h($h)."' title='".lang(72)."'>".($_GET["dbsize"]?$S:"?")."</a>","<td align='right' id='size-".h($h)."'>".($_GET["dbsize"]?db_size($h):"?"),"\n";}echo"</tbody>\n",script("mixin(qsl('tbody'), {onclick: tableClick, ondblclick: event => tableClick(event, true)});"),"</table>\n","</div>\n";if(support("database"))echo"<div class='table-footer'><div class='field-sets'>\n","<fieldset><legend>",lang(166)," <span id='selected'></span></legend><div class='fieldset-content'>\n",input_hidden("all"),script("qsl('input').onclick = countDbs;"),"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(),"\n","</div></fieldset>\n","</div></div>\n",script("initTableFooter()");echo"</div>\n",input_token(),"</form>\n",script("tableCheck();");}}echo'<p class="links"><a href="'.h(ME).'database=">'.icon("database-add").lang(76)."</a>\n";page_footer("db");exit;}if(isset($_GET["select"])&&($_POST["edit"]||$_POST["clone"])&&!$_POST["save"])$_GET["edit"]=$_GET["select"];if(isset($_GET["callf"]))$_GET["call"]=$_GET["callf"];if(isset($_GET["function"]))$_GET["procedure"]=$_GET["function"];if(isset($_GET["download"])){$a=$_GET["download"];$l=fields($a);header("Content-Type: application/octet-stream");header("Content-Disposition: attachment; filename=".friendly_url("$a-".implode("_",$_GET["where"])).".".friendly_url($_GET["field"]));$N=[idf_escape($_GET["field"])];$J=Driver::get()->select($a,$N,[where($_GET,$l)],$N);$L=($J?$J->fetchRow():[]);echo
Connection::get()->formatValue($L[0],$l[$_GET["field"]]);exit;}elseif(isset($_GET["table"])){$a=$_GET["table"];$l=fields($a);if(!$l)Admin::get()->addError(error()?:lang(79));$R=table_status1($a,true);$A=Admin::get()->getTableName($R);$mk=[];foreach($l
as$u=>$k)$mk+=$k["privileges"];$T=$l&&is_view($R)?$R['Engine']=='materialized view'?lang(168):lang(169):lang(9);$Ul=$A!=""?$A:h($a);page_header("$T: $Ul",[$Ul]);$Hf=null;if(isset($mk["insert"])||!support("table"))$Hf=[];Admin::get()->printTableMenu($R,$Hf);$_f=[];if(!preg_match("~sqlite|mssql|pgsql~",DIALECT)&&isset($R["Engine"]))$_f[]=lang(170).": ".h($R["Engine"]);if(isset($R["Collation"]))$_f[]=lang(47).": ".h($R["Collation"]);if($_f)echo"<p>",implode(", ",$_f),"</p>";if($l)Admin::get()->printTableStructure($l);$Ob=$R["Comment"];if($Ob!="")echo"<p class='keep-lines'>",lang(48),": ",Admin::get()->formatComment($Ob),"</p>\n";if(!is_view($R))$od='<a href="'.h(ME).'create='.urlencode($a).'">'.icon("edit").lang(36)."</a>";elseif(support("view"))$od='<a href="'.h(ME).'view='.urlencode($a).'">'.icon("edit").lang(37)."</a>";else$od="";if($od&&($_f||$l||$Ob!="")){echo"<p id='table-links' class='links'>",$od;$Al=Admin::get()->getStructureDumpFormats();if($Al){$hc=Admin::get()->getSettings()->getParameter("copyFormat");if(!isset($Al[$hc]))$hc=key($Al);echo"<span>","<a id='copy-table-structure' href='".h(ME)."dump=".urlencode($a)."&format=".urlencode($hc)."' class='jsonly'>".icon("copy").lang(171,h($Al[$hc]))."</a>";$Gk=count($Al)>1;if($Gk)echo"<a href='#format-list' class='toggle jsonly' title='".lang(172)."'>".icon_chevron_down()."</a>";echo"</span></p>\n";if($Gk){echo"<ul id='format-list' class='select-menu hidden'>";foreach($Al
as$ve=>$A)echo"<li><a href='".h(ME)."dump=".urlencode($a)."&format=".urlencode($ve)."'>".h($A)."</a></li>";echo"</ul>",script("initToggles(gid('table-links'));");}echo
script("initCopyTableStructure();");}else
echo"</p>\n";}$Ui=Driver::get()->getParentTables($a);if($Ui){echo"<h2>".lang(173)."</h2>\n";Admin::get()->printRelatedTables($Ui);}if(Driver::get()->getPartitionBy()&&str_contains(isset($R["Create_options"])?$R["Create_options"]:"","partitioned")){$dj=Driver::get()->getPartitionsInfo($a);if($dj){echo"<h2 id='partitions'>".lang(50)."</h2>\n";Admin::get()->printTablePartitions($dj);if(DIALECT!="pgsql"&&$od)echo"<p class='links'>$od</p>\n";}}$Af=Driver::get()->getInheritedTables($a);if($Af){echo"<h2 id='inherited-by'>".lang(174)."</h2>\n";Admin::get()->printRelatedTables($Af);}if(support("indexes")&&Driver::get()->supportsIndex($R)){echo"<h2 id='indexes'>".lang(175)."</h2>\n";$t=indexes($a);if($t)Admin::get()->printTableIndexes($t,$R);if(Driver::get()->supportsAlterIndex($R))echo'<p class="links"><a href="'.h(ME).'indexes='.urlencode($a).'">'.icon("edit").lang(176)."</a>\n";}if(!is_view($R)){if(fk_support($R)){echo"<h2 id='foreign-keys'>".lang(91)."</h2>\n";$te=foreign_keys($a);if($te){echo"<table>\n","<thead><tr><th>".lang(177)."<td>".lang(178)."<td>".lang(94)."<td>".lang(93)."<td></thead>\n";foreach($te
as$A=>$o)echo"<tr title='".h($A)."'>","<th><i>".implode("</i>, <i>",array_map('AdminNeo\h',$o["source"]))."</i>","<td><a href='".h($o["db"]!=""?preg_replace('~db=[^&]*~',"db=".urlencode($o["db"]),ME):($o["ns"]!=""?preg_replace('~ns=[^&]*~',"ns=".urlencode($o["ns"]),ME):ME))."table=".urlencode($o["table"])."'>".($o["db"]!=""&&$o["db"]!=DB?"<b>".h($o["db"])."</b>.":"").($o["ns"]!=""&&$o["ns"]!=$_GET["ns"]?"<b>".h($o["ns"])."</b>.":"").h($o["table"])."</a>","(<i>".implode("</i>, <i>",array_map('AdminNeo\h',$o["target"]))."</i>)","<td>".h($o["on_delete"]),"<td>".h($o["on_update"]),'<td><a href="'.h(ME.'foreign='.urlencode($a).'&name='.urlencode($A)).'">'.lang(179).'</a>',"\n";echo"</table>\n";}echo'<p class="links"><a href="'.h(ME).'foreign='.urlencode($a).'">'.icon("add").lang(180)."</a>\n";}if(support("check")){echo"<h2 id='checks'>".lang(181)."</h2>\n";$ub=Driver::get()->checkConstraints($a);if($ub){echo"<table cellspacing='0'>\n";foreach($ub
as$u=>$X)echo"<tr title='".h($u)."'>","<td><code class='jush-".DIALECT."'>".truncate_utf8(preg_replace('~\s+~',' ',ltrim($X))),"<td><a href='".h(ME.'check='.urlencode($a).'&name='.urlencode($u))."'>".lang(179)."</a>","\n";echo"</table>\n";}echo'<p class="links"><a href="'.h(ME).'check='.urlencode($a).'">'.icon("add").lang(182)."</a>\n";}}if(support(is_view($R)?"view_trigger":"trigger")){echo"<h2 id='triggers'>".lang(183)."</h2>\n";$Gm=triggers($a);if($Gm){echo"<table>\n";foreach($Gm
as$u=>$X)echo"<tr><td>".h($X[0])."<td>".h($X[1])."<th>".h($u)."<td><a href='".h(ME.'trigger='.urlencode($a).'&name='.urlencode($u))."'>".lang(179)."</a>\n";echo"</table>\n";}echo'<p class="links"><a href="'.h(ME).'trigger='.urlencode($a).'">'.icon("add").lang(184)."</a>\n";}}elseif(isset($_GET["schema"])){$tm=h(": ".DB.($_GET["ns"]?".$_GET[ns]":""));page_header(lang(72).$tm,[lang(72)]);$Wl=[];$Xl=[];$ce=[];$pa=($_GET["schema"]?:$_COOKIE["neo_schema-".str_replace(".","_",DB)]);preg_match_all('~([^:]+):([-0-9.]+)x([-0-9.]+)(_|$)~',$pa,$_,PREG_SET_ORDER);foreach($_
as$z){$Wl[$z[1]]=[(float)$z[2],(float)$z[3]];$Xl[]="\n'".js_escape($z[1])."': [ $z[2], $z[3] ]";}$Jg=1.4;$zm=0;$Ya=-1;$_k=[];$Zj=[];$Bg=[];$Fa=Driver::get()->getAllFields();foreach(table_status('',true)as$Q=>$R){if(is_view($R))continue;$H=0;$_k[$Q]["fields"]=[];foreach(isset($Fa[$Q])?$Fa[$Q]:[]as$k){$H=round($H+$Jg,2);$ce[$Q][$k["field"]]=$H;$_k[$Q]["fields"][$k["field"]]=$k;}$_k[$Q]["pos"]=(isset($Wl[$Q])?$Wl[$Q]:[$zm,0]);foreach(Admin::get()->getForeignKeys($Q)as$X){if(!$X["db"]){$_g=$Ya;if((isset($Wl[$Q][1])?$Wl[$Q][1]:0)||(isset($Wl[$X["table"]][1])?$Wl[$X["table"]][1]:0))$_g=min(floatval(isset($Wl[$Q][1])?$Wl[$Q][1]:0),floatval(isset($Wl[$X["table"]][1])?$Wl[$X["table"]][1]:0))-1;else$Ya-=.1;while($Bg[(string)$_g])$_g-=.0001;$_k[$Q]["references"][$X["table"]][(string)$_g]=[$X["source"],$X["target"]];$Zj[$X["table"]][$Q][(string)$_g]=$X["target"];$Bg[(string)$_g]=true;}}$zm=max($zm,round($_k[$Q]["pos"][0]+$Jg+$H+1,2));}echo"<div id='schema' style='height: {$zm}em;'>\n";foreach($_k
as$A=>$Q){$bf=round($Jg+count($Q["fields"])*$Jg+0.4,2);echo"<div class='table' style='top: ".$Q["pos"][0]."em; left: ".$Q["pos"][1]."em; height: {$bf}em;'>","<div class='content'>",'<h4><a href="'.h(ME).'table='.urlencode($A).'">'.h($A)."</a></h4>","<ul>";foreach($Q["fields"]as$k){$X='<span '.type_class($k["type"]).' title="'.h($k["type"].($k["length"]?"($k[length])":"").($k["null"]?" NULL":'')).'">'.h($k["field"]).'</span>';echo"<li>".($k["primary"]?"<i>$X</i>":$X)."</li>";}echo"</ul>","</div>";foreach((array)$Q["references"]as$hm=>$bk){foreach($bk
as$_g=>$Vj){$Ag=$_g-(isset($Wl[$A][1])?$Wl[$A][1]:0);$q=0;foreach($Vj[0]as$kl){echo"\n<div class='references outgoing' title='",h($hm),"' id='refs$_g-$q' style='left: {$Ag}em; top: ",$ce[$A][$kl],"em;'>","<div style='width: ".(-$Ag)."em;'></div>","</div>";$q++;}}}foreach((array)$Zj[$A]as$hm=>$bk){foreach($bk
as$_g=>$c){$Ag=$_g-(isset($Wl[$A][1])?$Wl[$A][1]:0);$q=0;foreach($c
as$gm){echo"\n<div class='references incoming' title='",h($hm),"' id='refd$_g-$q' style='left: {$Ag}em; top: ".$ce[$A][$gm]."em;'>","<svg viewBox='0 0 22 22' fill='currentColor'><path d='M11,19l10,-8l-10,-8l0,16Z'/></svg>","<div style='width: ".(-$Ag)."em;'></div>","</div>";$q++;}}}echo"\n</div>\n";}foreach($_k
as$A=>$Q){foreach((array)$Q["references"]as$hm=>$bk){if($_k[$hm]){foreach($bk
as$_g=>$Vj){$wh=$zm;$fh=-10;foreach($Vj[0]as$u=>$kl){$tj=$Q["pos"][0]+$ce[$A][$kl];$uj=$_k[$hm]["pos"][0]+$ce[$hm][$Vj[1][$u]];$wh=round(min($wh,$tj,$uj),2);$fh=round(max($fh,$tj,$uj),2);}echo"<div class='references vertical' id='refl$_g' style='left: $_g"."em; top: $wh"."em;'>"."<div style='height: ".round($fh-$wh,2)."em;'></div></div>\n";}}}}echo"</div>\n",script("initSchema('".js_escape(DB)."', $zm, {".implode(",",$Xl)."})"),"<p class='links'>","<a href='",h(ME."schema=".urlencode($pa)),"' id='schema-link'>",lang(185),"</a>","</p>\n";}elseif(isset($_GET["dump"])){$a=$_GET["dump"];$P=Admin::get()->getSettings();$Al=Admin::get()->getStructureDumpFormats();$tc=Admin::get()->getDataDumpFormats();$ic=is_ajax()&&$a!="";if($ic){$hc=isset($_GET["format"])?$_GET["format"]:null;if(!isset($Al[$hc])){reset($Al);$hc=key($Al);}$P->updateParameter("copyFormat",$hc);$_POST=["format"=>$hc,"output"=>"text","db_style"=>"","table_style"=>"CREATE","tables"=>[$a],"triggers"=>(support("trigger")?"1":""),"data_style"=>"",];}if($_POST){$ld=isset($Al[$_POST["format"]]);$kd=isset($tc[$_POST["format"]]);if(!$ic)$P->updateParameters(["dumpFormat"=>$_POST["format"],"dumpOutput"=>$_POST["output"],]+($ld?["dumpDbStyle"=>$_POST["db_style"],"dumpTypes"=>isset($_POST["types"])?$_POST["types"]:(support("type")?"":null),"dumpRoutines"=>isset($_POST["routines"])?$_POST["routines"]:(support("routine")?"":null),"dumpEvents"=>isset($_POST["events"])?$_POST["events"]:(support("event")?"":null),"dumpTableStyle"=>$_POST["table_style"],"dumpAutoIncrement"=>isset($_POST["auto_increment"])?$_POST["auto_increment"]:"","dumpTriggers"=>isset($_POST["triggers"])?$_POST["triggers"]:(support("trigger")?"":null),]:[])+($kd?["dumpDataStyle"=>$_POST["data_style"],]:[]));$Zl=($ld?$_POST["table_style"]:"CREATE");$vc=($kd?$_POST["data_style"]:"");if(DB!="")$g=[DB];else{$g=isset($_POST["databases"])?$_POST["databases"]:[];if(is_string($g))$g=explode("\n",rtrim(str_replace("\r","",$g),"\n"));}$Ak=isset($_POST["schemas"])?$_POST["schemas"]:[];$S=array_flip(isset($_POST["tables"])?$_POST["tables"]:[])+array_flip(isset($_POST["data"])?$_POST["data"]:[]);if(count($S)==1)$of=key($S);elseif(count($Ak)==1)$of=$Ak[0];elseif(count($g)==1)$of=$g[0];else$of=Admin::get()->getServerName(SERVER,true,"server");$Qd=dump_headers($of,DB==""||$_GET["ns"]===""||count($S)>1);$Zf=preg_match('~sql~',$_POST["format"]);$uc=$Zf&&$vc&&!$Zl&&DIALECT!="sql";if($Zf){if(!$ic)echo"-- AdminNeo ".VERSION." ".Drivers::get(DRIVER)." ".Connection::get()->getVersion()." dump\n\n";if(DIALECT=="sql"){if(!$ic)echo"SET NAMES utf8;
SET time_zone = '+00:00';
SET foreign_key_checks = 0;
".($vc?"SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO';
":"")."
";Connection::get()->query("SET time_zone = '+00:00'");Connection::get()->query("SET sql_mode = ''");}}$Cl=($ld?$_POST["db_style"]:"");foreach($g
as$h){if(Connection::get()->selectDatabase($h)){Admin::get()->dumpDatabase($h);if($Zf){if($Cl)echo
create_database_sql($h,$Cl),use_sql($h,$Cl)."\n";$Ni="";if($_POST["types"]){foreach(types()as$r=>$U){$Fc=type_definition($r);$ci=($Fc["kind"]=='d'?"DOMAIN":"TYPE");if($Fc["definition"])$Ni
.=($Cl!='DROP+CREATE'?"DROP $ci IF EXISTS ".idf_escape($U).";;\n":"")."CREATE $ci ".idf_escape($U)." $Fc[definition];\n\n";else$Ni
.="-- Could not export type $U\n\n";}}if($_POST["routines"]){foreach(routines()as$L){$A=$L["ROUTINE_NAME"];$ok=$L["ROUTINE_TYPE"];$e=create_routine($ok,["name"=>$A]+routine($L["SPECIFIC_NAME"],$ok));set_utf8mb4($e);$Ni
.=($Cl!='DROP+CREATE'?"DROP $ok IF EXISTS ".idf_escape($A).";;\n":"")."$e;\n\n";}}if($_POST["events"]){foreach(get_rows("SHOW EVENTS",null,"-- ")as$L){$e=remove_definer(Connection::get()->getValue("SHOW CREATE EVENT ".idf_escape($L["Name"]),3));set_utf8mb4($e);$Ni
.=($Cl!='DROP+CREATE'?"DROP EVENT IF EXISTS ".idf_escape($L["Name"]).";;\n":"")."$e;;\n\n";}}echo($Ni&&DIALECT=='sql'?"DELIMITER ;;\n\n$Ni"."DELIMITER ;\n\n":$Ni);}if($Zl||$vc){foreach(($_GET["ns"]===""?(array)$_POST["schemas"]:(DB!=""||!support("scheme")?[""]:Admin::get()->getSchemas(true)))as$_k){if($_k!="")set_schema($_k);$cm=table_status('',true);$Vl=array_keys($cm);$Rc=false;if($uc&&$Vl){$ak=[];foreach($Vl
as$A){if(!is_view($cm[$A])&&(DB==""||$_GET["ns"]===""||in_array($A,(array)$_POST["data"]))){foreach(foreign_keys($A)as$o)$ak[$A][]=$o["table"];}}$Ci=dump_table_order($Vl,$ak);if($Ci)$Vl=$Ci;else$Rc=function_exists('AdminNeo\foreign_key_checks_sql');}if($Rc)echo
foreign_key_checks_sql(false)."\n";$un=[];foreach($Vl
as$A){$R=$cm[$A];$Q=(DB==""||$_GET["ns"]===""||in_array($A,(array)$_POST["tables"]));$f=(DB==""||$_GET["ns"]===""||in_array($A,(array)$_POST["data"]));if($Q||$f){$wm=null;if($Qd=="tar"){$wm=new
TmpFile();ob_start([$wm,'write'],1e5);}$kc=($Q||!$ld?$Zl:"");Admin::get()->dumpTable($A,$kc,(is_view($R)?2:0));if(is_view($R)&&$Qd!="tar")$un[]=$A;elseif($f){$l=fields($A);Admin::get()->dumpData($A,$vc,"SELECT *".convert_fields($l,$l)." FROM ".table($A));if($Zf&&!$kc&&$_POST["auto_increment"]&&function_exists('AdminNeo\restart_sequences_sql'))echo"\n".restart_sequences_sql($A);}if($Zf&&$_POST["triggers"]&&$Q&&($Gm=trigger_sql($A)))echo"\nDELIMITER ;;\n$Gm\nDELIMITER ;\n";if($Qd=="tar"){ob_end_flush();tar_file((DB!=""?"":"$h/")."$A.csv",$wm);}elseif($Zf)echo"\n";}}if($Rc)echo
foreign_key_checks_sql(true)."\n";if($Zf&&$Zl&&function_exists('AdminNeo\foreign_keys_sql')){foreach($cm
as$A=>$R){$Q=(DB==""||$_GET["ns"]===""||in_array($A,(array)$_POST["tables"]));if($Q&&!is_view($R))echo
foreign_keys_sql($A);}}foreach($un
as$sn)Admin::get()->dumpTable($sn,$Zl,1);if($Qd=="tar")echo
pack("x512");}}}}if($Zf&&!$ic)echo"-- ".gmdate("Y-m-d H:i:s e")."\n";exit;}$A=DB!=""?h(DB):h(Admin::get()->getServerName(SERVER));page_header(lang(75).": $A",($_GET["export"]!=""?["table"=>$_GET["export"]]:[lang(75)]));echo"<form id='dump-form' action='' method='post'>\n","<table class='box'>\n";$zc=['','USE','DROP+CREATE','CREATE'];$Zl=['','DROP+CREATE','CREATE'];$vc=['','TRUNCATE+INSERT','INSERT'];if(DIALECT=="sql")$vc[]='INSERT+UPDATE';echo"<tr><th>",lang(172),"</th><td>",html_radios("format",$Al+$tc,$P->getParameter("dumpFormat","sql")),"</td></tr>\n";if(DIALECT!="sqlite"){echo"<tr><th id='label-db'>",lang(31),"</th>","<td>",html_select('db_style',$zc,$P->getParameter("dumpDbStyle",DB==""?"CREATE":""),"","label-db"),"<span class='labels'>";if(support("routine"))echo
checkbox("routines",1,$P->getParameter("dumpRoutines",$_GET["dump"]==""?"1":""),lang(186));if(support("event"))echo
checkbox("events",1,$P->getParameter("dumpEvents",$_GET["dump"]==""?"1":""),lang(187));echo"</span></td></tr>";}echo"<tr><th id='label-tables'>",lang(163),"</th><td>",html_select('table_style',$Zl,$P->getParameter("dumpTableStyle","DROP+CREATE"),"","label-tables")," <span class='labels'>",checkbox("auto_increment",1,$P->getParameter("dumpAutoIncrement"),lang(49));if(support("trigger"))echo
checkbox("triggers",1,$P->getParameter("dumpTriggers","1"),lang(183));echo"</span></td></tr>","<tr><th id='label-data'>",lang(188),"</th><td>",html_select("data_style",$vc,$P->getParameter("dumpDataStyle","INSERT"),"","label-data"),"</td></tr>","<tr><th>",lang(189),"</th><td>",html_radios("output",Admin::get()->getDumpOutputs(),$P->getParameter("dumpOutput","file")),"</td></tr>\n","</table>\n",script("initDumpFormats(".json_encode(array_keys($Al)).", ".json_encode(array_keys($tc)).");"),"<p>","<input type='submit' class='button default' value='",lang(75),"'>",input_token(),"</p>\n","<table>\n",script("qsl('table').onclick = dumpClick;");$zj=[];if(DB!=""&&$_GET["ns"]===""){echo"<thead><tr><th>","<label class='block'><input type='checkbox' id='check-schemas' checked class='jsonly' title='".lang(190)."'>".lang(191)."</label>".script("gid('check-schemas').onclick = partial(formCheck, /^schemas\\[/);",""),"</thead>\n";foreach(Admin::get()->getSchemas()as$_k){if(!information_schema(DB,$_k))echo"<tr><td>".checkbox("schemas[]",$_k,true,$_k,"","block")."\n";}}elseif(DB!=""){$wb=($a!=""?"":" checked");echo"<thead><tr>","<th><label class='block'><input type='checkbox' id='check-tables'$wb class='jsonly' title='".lang(190)."'>".lang(9)."</label>".script("gid('check-tables').onclick = partial(formCheck, /^tables\\[/);",""),"<th class='right'><label class='block'>".lang(188)."<input type='checkbox' id='check-data'$wb class='jsonly' title='".lang(190)."'></label>".script("gid('check-data').onclick = partial(formCheck, /^data\\[/);",""),"</thead>\n";$un="";$bm=tables_list();foreach($bm
as$A=>$U){$yj=preg_replace('~_.*~','',$A);$wb=($a==""||$a==(substr($a,-1)=="%"?"$yj%":$A));$Bj="<tr><td>".checkbox("tables[]",$A,$wb,$A,"","block");if($U!==null&&!preg_match('~table~i',$U))$un
.="$Bj\n";else
echo"$Bj<td class='right'><label class='block'><span id='Rows-".h($A)."'></span>".checkbox("data[]",$A,$wb)."</label>\n";$zj[$yj]++;}echo$un;if($bm)echo
script("ajaxSetHtml('".js_escape(ME)."script=db');");}else{$g=Admin::get()->getDatabases();echo"<thead><tr><th>","<label class='block'>".($g?"<input type='checkbox' id='check-databases'".($a==""?" checked":"")." class='jsonly' title='".lang(190)."'>".script("gid('check-databases').onclick = partial(formCheck, /^databases\\[/);",""):"").lang(31)."</label>","</thead>\n";if($g){foreach($g
as$h){if(!information_schema($h)){$yj=preg_replace('~_.*~','',$h);echo"<tr><td>".checkbox("databases[]",$h,$a==""||$a=="$yj%",$h,"","block")."\n";$zj[$yj]++;}}}else
echo"<tr><td><textarea name='databases' rows='10' cols='20'></textarea>";}echo"</table>\n","</form>\n";$Mg=[];foreach($zj
as$u=>$X){if($u!=""&&$X>1)$Mg[]="<a href='".h(ME)."dump=".urlencode("$u%")."'>".icon("check").h($u)."*</a>";}if($Mg)echo"<p class='links'>",implode("",$Mg),"</p>\n";}elseif(isset($_GET["privileges"])){$tm=DB!=""?h(": ".DB):"";page_header(lang(73).$tm,[lang(73)]);echo'<p class="links top-links"><a href="',h(ME),'user=">',icon("user-add"),lang(192),"</a></p>\n";$J=Connection::get()->query("SELECT User, Host FROM mysql.".(DB==""?"user":"db WHERE ".q(DB)." LIKE Db")." ORDER BY Host, User");$Ke=$J;if(!$J)$J=Connection::get()->query("SELECT SUBSTRING_INDEX(CURRENT_USER, '@', 1) AS User, SUBSTRING_INDEX(CURRENT_USER, '@', -1) AS Host");echo"<form action=''>\n";hidden_fields_get();echo
input_hidden("db",DB);if(!$Ke)echo
input_hidden("grant");echo"\n","<div class='scrollable'>\n","<table class='checkable'>\n","<thead><tr><th>".lang(6)."<th>".lang(5)."<th></thead>\n";while($L=$J->fetchAssoc())echo'<tr><td>'.h($L["User"])."<td>".h($L["Host"]).'<td><a href="'.h(ME.'user='.urlencode($L["User"]).'&host='.urlencode($L["Host"])).'">'.lang(39)."</a>\n";if(!$Ke||DB!="")echo"<tr><td><input class='input' name='user' autocapitalize='off'>"."<td><input class='input' name='host' value='localhost' autocapitalize='off'>"."<td><input type='submit' class='button' value='".lang(39)."'>\n";echo"</table>\n","</div>\n","</form>\n";}elseif(isset($_GET["sql"])){function
get_unsupported_query($I,$ml){if(DIALECT!="sqlite"||preg_match("~^(?:$ml|;)*+(ATTACH|VACUUM\\b.*\\bINTO)\\b~is",$I,$z)===0)return
null;return
preg_match('~ATTACH~i',isset($z[1])?$z[1]:"")?'ATTACH':'VACUUM INTO';}$P=Admin::get()->getSettings();$Ig=Driver::get()->lineComment();$ml="(?:\\s|\xEF\xBB\xBF|/\\*[\s\S]*?\\*/|(?:$Ig)[^\n]*\n?|--\r?\n)";if($_POST["export"]&&!get_unsupported_query($_POST["query"],$ml)){$P->updateParameters(["exportFormat"=>$_POST["format"],"exportOutput"=>$_POST["output"],]);dump_headers("sql");Admin::get()->dumpTable("","");Admin::get()->dumpData("","table",$_POST["query"]);exit;}restart_session();$gf=&get_session("queries");$ff=&$gf[DB];if($_POST["clear"]){$ff=[];redirect(remove_from_uri("history"));}stop_session();$T=isset($_GET["import"])?lang(74):lang(41);page_header($T,[$T]);if($_POST){$Ae=false;if(!isset($_GET["import"]))$I=$_POST["query"];elseif($_POST["webfile"]){$sf=Admin::get()->getImportFilePath();if($sf){if(file_exists($sf))$Ae=fopen($sf,"rb");elseif(file_exists("$sf.gz"))$Ae=fopen("compress.zlib://$sf.gz","rb");}$I=$Ae?fread($Ae,1e6):false;}else$I=get_file("sql_file",true,";");if(is_string($I)){if(($lh=ini_bytes("memory_limit"))!="-1")ini_set("memory_limit",max($lh,strval(2*strlen($I)+memory_get_usage()+8e6)));if($I!=""&&strlen($I)<1e6){$Jj=$I.(preg_match("~;[ \t\r\n]*\$~",$I)?"":";");if(!$ff||first(end($ff))!=$Jj){restart_session();$ff[]=[$Jj,time()];set_session("queries",$gf);stop_session();}}$Hc=";";$Ic=1;$di=0;$ud=true;$gc=false;$Yb=connect();if($Yb&&DB!=""){$Yb->selectDatabase(DB);if($_GET["ns"]!="")set_schema($_GET["ns"],$Yb);}$Nb=0;$Dd=[];$Vi='[\'"'.(DIALECT=="sql"?'`':(DIALECT=="sqlite"?'`[':(DIALECT=="mssql"?'[':''))).']|/\*|'.$Ig.'|$'.(DIALECT=="pgsql"?'|\$([a-zA-Z]\w*)?\$':'');$_m=microtime(true);$tc=Admin::get()->getDataDumpFormats();unset($tc["sql"]);while($I!=""){if(!$di&&preg_match("~^$ml*+DELIMITER\\s+(\\S+)~i",$I,$z)){$Hc=preg_quote($z[1]);$Ic=strlen($z[1]);$xe=Admin::get()->formatSqlCommandQuery(trim($z[0]));if($xe!="")echo"<pre><code class='jush-".DIALECT."'>$xe</code></pre>\n";$I=substr($I,strlen($z[0]));}elseif(!$di&&DIALECT=="pgsql"&&preg_match("~^($ml*+COPY\\s+)[^;]+\\s+FROM\\s+stdin;~i",$I,$z)){$Hc="\n\\\\\\.\r?\n";$Ic=3;$gc=true;$di=strlen($z[0]);}else{preg_match("($Hc\\s*|$Vi)",$I,$z,PREG_OFFSET_CAPTURE,$di);list($ze,$H)=$z[0];if(!$ze&&$Ae&&!feof($Ae))$I
.=fread($Ae,1e5);else{if(!$ze&&rtrim($I)=="")break;$di=$H+strlen($ze);if($ze&&!preg_match("(^$Hc)",$ze)){$kb=Driver::get()->hasCStyleEscapes()||(DIALECT=="pgsql"&&($H>0&&strtolower($I[$H-1])=="e"));$ij='(';if($ze=='/*')$ij
.='\*/';elseif($ze=='[')$ij
.=']';elseif(preg_match("~^(?:$Ig)~",$ze))$ij
.="\n";else$ij
.=preg_quote($ze).($kb?"|\\\\.":"");$ij
.='|$)s';while(preg_match($ij,$I,$z,PREG_OFFSET_CAPTURE,$di)){$sk=$z[0][0];if(!$sk&&$Ae&&!feof($Ae))$I
.=fread($Ae,1e5);else{$di=$z[0][1]+strlen($sk);if(!isset($sk[0])||$sk[0]!="\\")break;}}}elseif(DIALECT=="sqlite"&&preg_match("~^(?:$ml|;)*+\$~",substr($I,0,$H+$Ic))){$I=substr($I,$di);$di=0;}else{$ud=false;$Jj=substr($I,0,$H+$Ic);$Nb++;$Bj="<pre id='sql-$Nb'><code class='jush-".DIALECT."'>".Admin::get()->formatSqlCommandQuery(trim($Jj))."</code></pre>\n";if($Wm=get_unsupported_query($Jj,$ml)){echo$Bj,"<p class='error'>".lang(193,$Wm)."\n";$Dd[]=" <a href='#sql-$Nb'>$Nb</a>";if($_POST["error_stops"])break;}else{if(!$_POST["only_errors"]){echo$Bj;ob_flush();flush();}$tl=microtime(true);if(Connection::get()->multiQuery($Jj)&&is_object($Yb)&&preg_match("~^$ml*+USE\\b~i",$Jj))$Yb->query($Jj);do{$J=Connection::get()->storeResult();if(Connection::get()->getError()){echo($_POST["only_errors"]?$Bj:""),"<p class='error'>",lang(194),(!empty(Connection::get()->getErrno())?" (".Connection::get()->getErrno().")":""),": ",error()."</p>\n";$Dd[]=" <a href='#sql-$Nb'>$Nb</a>";if($_POST["error_stops"])break
2;}else{$qm=" <span class='time'>(".format_time($tl).")</span>";$pd=(strlen($Jj)<1000?" <a href='".h(ME)."sql=".urlencode(trim($Jj))."'>".icon("edit").lang(39)."</a>":"");$Nj=Connection::get()->getQueryInfo();$za=Connection::get()->getAffectedRows();$Bh=Connection::get()->hasMoreResults();$_n=($_POST["only_errors"]||$Bh?null:Driver::get()->warnings());$Bn=($Bh?Connection::get()->getWarningsCount():0);$Cn="warnings-$Nb";if($_n)$Dn="<a href='#$Cn' class='toggle'>".lang(40).icon_chevron_down()."</a>";elseif($Bn)$Dn="<span>".lang(40).": $Bn</span>";else$Dn=null;$Ld=$Fi=null;$Md="explain-$Nb";$Nd=false;$Od="export-$Nb";$w=0;if(is_object($J)){if(!$_POST["only_errors"])echo"<div class='table-result'>\n";$w=(int)$_POST["limit"];$Fi=print_select_result($J,$Yb,[],$w);if(!$_POST["only_errors"]){echo"<p class='links'>";$Zh=$J->getRowsCount();echo($Zh?($w&&$Zh>$w?lang(195,$w):"").lang(196,$Zh):""),$qm,$pd,$Dn;if($Yb&&preg_match("~^($ml|\\()*+SELECT\\b~i",$Jj)&&($Ld=explain($Yb,$Jj)))echo"<a href='#$Md' class='toggle'>Explain".icon_chevron_down()."</a>";$Nd=true;echo"<a href='#$Od' class='toggle'>".lang(75).icon_chevron_down()."</a>","</p>\n";}}else{if(preg_match("~^$ml*+(CREATE|DROP|ALTER)$ml++(DATABASE|SCHEMA)\\b~i",$Jj)){restart_session();set_session("dbs",null);stop_session();}if(!$_POST["only_errors"]){echo"<p class='message' title='".h($Nj)."'>",lang(197,$za),"$qm $pd";if($Dn)echo", $Dn";echo"</p>\n";}}if(!$_POST["only_errors"])echo
script("initToggles(qsl('p'));");if($_n)echo"<div id='$Cn' class='hidden'>\n$_n</div>\n";if($Ld){echo"<div id='$Md' class='hidden explain'>\n";print_select_result($Ld,$Yb,$Fi);echo"</div>\n";}if($Nd){echo"<form id='$Od' action='' method='post' class='hidden'><p>\n",html_select("format",$tc,$P->getParameter("exportFormat")),html_select("output",Admin::get()->getDumpOutputs(),$P->getParameter("exportOutput"))." ",input_hidden("query",$Jj),input_token()," <input type='submit' class='button' name='export' value='".lang(75)."'>";if(!$w)echo
script("qsl('input').onclick = function (event) { return sqlExport.call(this, event, '".js_escape(ME)."set=export-settings'); };","");echo"</p></form>\n";}if(is_object($J)&&!$_POST["only_errors"])echo"</div>\n";}$tl=microtime(true);}while(Connection::get()->nextResult());}$I=substr($I,$di);$di=0;if($gc){$Hc=";";$Ic=1;$gc=false;}}}}}if($ud)echo"<p class='message'>".lang(198)."\n";else{$vf=Connection::get()->inTransaction();Driver::get()->rollback();if($vf)echo"<pre><code class='jush-".DIALECT."'>ROLLBACK -- AdminNeo</code></pre>\n";if($_POST["only_errors"]){$gi=$Nb-count($Dd);echo"<p class='".($gi?"message":"error")."'>".lang(199,$Nb-count($Dd))," <span class='time'>(".format_time($_m).")</span>\n";}elseif($Dd&&$Nb>1)echo"<p class='error'>".lang(194).": ".implode("",$Dd)."\n";}}else
echo"<p class='error'>".upload_error($I)."\n";}echo"<form action='' method='post' enctype='multipart/form-data' id='form'>\n";if(!isset($_GET["import"])){$Jj=$_GET["sql"];if($_POST)$Jj=$_POST["query"];elseif($_GET["history"]=="all")$Jj=$ff;elseif($_GET["history"]!="")$Jj=$ff[$_GET["history"]][0];echo"<p>";textarea("query",$Jj,20);echo
script(($_POST?"":"qs('textarea').focus();\n")."gid('form').onsubmit = partial(sqlSubmit, gid('form'), '".js_escape(remove_from_uri("sql|limit|error_stops|only_errors|history"))."');"),"</p>","<p><input type='submit' class='button default' value='".lang(200)."' title='Ctrl+Enter'>",lang(201).": <input type='number' name='limit' class='input size' value='".h($_POST?$_POST["limit"]:$_GET["limit"])."'>\n";}else{echo"<div class='field-sets'>\n","<fieldset><legend>".lang(202)."</legend><div class='fieldset-content'>";$Se=(extension_loaded("zlib")?"[.gz]":"");if(ini_bool("file_uploads"))echo"SQL$Se (&lt; ".ini_get("upload_max_filesize")."B): <input type='file' name='sql_file[]' multiple>","<input type='submit' class='button default' value='".lang(200)."'>",file_upload_form_script("form","sql_file[]");else
echo
lang(203);echo"</div></fieldset>\n";$sf=Admin::get()->getImportFilePath();if($sf)echo"<fieldset><legend>".lang(204)."</legend><div class='fieldset-content'>",lang(205,"<code>".h($sf)."$Se</code>")," <input type='submit' class='button default' name='webfile' value='".lang(206)."'>","</div></fieldset>\n";echo"</div>\n","<p>";}echo
checkbox("error_stops",1,($_POST?$_POST["error_stops"]:(isset($_GET["error_stops"])?$_GET["error_stops"]:true)),lang(207)),checkbox("only_errors",1,($_POST?$_POST["only_errors"]:isset($_GET["import"])||$_GET["only_errors"]),lang(208)),input_token(),"</p>\n";if(!isset($_GET["import"]))Admin::get()->printAfterSqlCommand();if(!isset($_GET["import"])&&$ff){echo"<div class='field-sets'>\n";print_fieldset_start("history",lang(209),"history",$_GET["history"]!="");for($X=end($ff);$X;$X=prev($ff)){$u=key($ff);list($Jj,$qm,$td)=$X;echo" <pre><code class='jush-".DIALECT."'>",truncate_utf8(preg_replace('~\s+~',' ',ltrim(preg_replace("~^(?:$Ig).*~m",'',$Jj)))),"</code></pre>",'<p class="links">',"<a href='".h(ME."sql=&history=$u")."'>".icon("edit").lang(39)."</a>"," <span class='time' title='".@date('Y-m-d',$qm)."'>".@date("H:i:s",$qm).($td?" ($td)":"")."</span>","</p>";}echo"<p><input type='submit' class='button' name='clear' value='".lang(210)."'>\n","<a href='",h(ME."sql=&history=all")."' class='button light'>",icon("edit"),lang(211),"</a></p>\n";print_fieldset_end("history");echo"</div>\n";}echo"</form>\n";}elseif(isset($_GET["edit"])){$a=$_GET["edit"];$l=fields($a);$Z=(isset($_GET["select"])?($_POST["check"]&&count($_POST["check"])==1?where_check($_POST["check"][0],$l):""):where($_GET,$l));$Ym=(isset($_GET["select"])?$_POST["edit"]:$Z);foreach($l
as$A=>$k){if((!$Ym&&!isset($k["privileges"]["insert"]))||Admin::get()->getFieldName($k)=="")unset($l[$A]);}if($_POST&&!isset($_GET["select"])){$y=$_POST["referer"];if($_POST["insert"])$y=($Ym?null:$_SERVER["REQUEST_URI"]);elseif(!preg_match('~^.+&select=.+$~',$y)||!is_same_host($y))$y=ME."select=".urlencode($a);$t=indexes($a);$Qm=unique_array(isset($_GET["where"])?$_GET["where"]:[],$t);$Oj="\nWHERE $Z";if(isset($_POST["delete"]))queries_redirect($y,lang(212),(bool)Driver::get()->delete($a,$Oj,$Qm?0:1));else{$Yk=[];foreach($l
as$A=>$k){$X=process_input($k);if($X!==false&&$X!==null)$Yk[idf_escape($A)]=$X;}if($Ym){if(!$Yk)redirect($y);queries_redirect($y,lang(213),(bool)Driver::get()->update($a,$Yk,$Oj,$Qm?0:1));if(is_ajax()){page_headers();page_messages();exit;}}else{$J=Driver::get()->insert($a,$Yk);$yg=($J?last_id($J):0);queries_redirect($y,lang(214,($yg?" $yg":"")),(bool)$J);}}}$L=null;if($Z){$N=[];foreach($l
as$A=>$k){if(isset($k["privileges"]["select"])){$Ma=($_POST["clone"]&&$k["auto_increment"]?"''":convert_field($k));$N[]=($Ma?"$Ma AS ":"").idf_escape($A);}}$L=[];if(!support("table"))$N=["*"];if($N){$J=Driver::get()->select($a,$N,[$Z],$N,[],(isset($_GET["select"])?2:1));if(!$J)Admin::get()->addError(error());else{$L=$J->fetchAssoc();if(!$L)$L=false;}if(isset($_GET["select"])&&(!$L||$J->fetchAssoc()))$L=null;}}if(!support("table")&&!$l){if(!$Z){$J=Driver::get()->select($a,["*"],[],["*"]);$L=($J?$J->fetchAssoc():false);if(!$L)$L=[Driver::get()->primary=>""];}if($L){foreach($L
as$u=>$X){if(!$Z)$L[$u]=null;$l[$u]=["field"=>$u,"null"=>($u!=Driver::get()->primary),"auto_increment"=>($u==Driver::get()->primary)];}}}if(isset($_POST["save"])?$_POST["save"]:false){$vj=[];foreach((isset($_POST["fields"])?$_POST["fields"]:[])as$u=>$X)$vj[bracket_escape($u,true)]=$X;$L=$vj+($L?:[]);}if($_POST["edit"]){$rd=array_filter($l,function($k){return!(isset($k["generated"])?$k["generated"]:null);});}else$rd=$l;edit_form($a,$rd,$L,$Ym);}elseif(isset($_GET["create"])){$a=$_GET["create"];$Zi=Driver::get()->getPartitionBy();$dj=$Zi?Driver::get()->getPartitionsInfo($a):[];$Xj=referencable_primary($a);$te=[];foreach($Xj
as$Ul=>$k)$te[str_replace("`","``",$Ul)."`".str_replace("`","``",$k["field"])]=$Ul;$Ii=[];$R=[];if($a!=""){$Ii=fields($a);$R=table_status1($a);if(count($R)<2)Admin::get()->addError(lang(79));}$L=$_POST;$L["Comment"]=normalize_newlines($L["Comment"]);$L["fields"]=(array)$L["fields"];if($L["auto_increment_col"])$L["fields"][$L["auto_increment_col"]]["auto_increment"]=true;if($_POST&&!Admin::get()->getErrors())Admin::get()->getSettings()->updateParameter("commentsOpened",isset($_POST["comments"])?$_POST["comments"]:null);if($_POST&&!process_fields($L["fields"])&&!Admin::get()->getErrors()){if($_POST["drop"])queries_redirect(substr(ME,0,-1),lang(215),drop_tables([$a]));else{$l=[];$Fa=[];$dn=false;$re=[];$Hi=reset($Ii);$Aa=" FIRST";foreach($L["fields"]as$u=>$k){$o=$te[$k["type"]];$Jm=($o!==null?$Xj[$o]:$k);if($k["field"]!=""){if(!$k["generated"])$k["default"]=null;$Hj=process_field($k,$Jm);$Fa[]=[$k["orig"],$Hj,$Aa];if(!$Hi||$Hj!==process_field($Hi,$Hi)){$l[]=[$k["orig"],$Hj,$Aa];if($k["orig"]!=""||$Aa)$dn=true;}if($o!==null)$re[idf_escape($k["field"])]=($a!=""&&DIALECT!="sqlite"?"ADD":" ").format_foreign_key(['table'=>$te[$k["type"]],'source'=>[$k["field"]],'target'=>[$Jm["field"]],'on_delete'=>$k["on_delete"],]);$Aa=" AFTER ".idf_escape($k["field"]);}elseif($k["orig"]!=""){$dn=true;$l[]=[$k["orig"]];}if($k["orig"]!=""){$Hi=next($Ii);if(!$Hi)$Aa="";}}$bj=[];if(in_array($L["partition_by"],$Zi)){foreach($L
as$u=>$X){if(preg_match('~^partition~',$u))$bj[$u]=$X;}foreach($bj["partition_names"]as$u=>$A){if($A===""){unset($bj["partition_names"][$u]);unset($bj["partition_values"][$u]);}}$bj["partition_names"]=array_values($bj["partition_names"]);$bj["partition_values"]=array_values($bj["partition_values"]);if($bj==$dj)$bj=[];}elseif(str_contains(isset($R["Create_options"])?$R["Create_options"]:"","partitioned"))$bj=null;$nh=lang(216);if($a==""){cookie("neo_engine",isset($L["Engine"])?$L["Engine"]:"");$nh=lang(217);}$A=trim($L["name"]);$y=ME.(support("table")?"table=":"select=").urlencode($A);$J=alter_table($a,$A,(DIALECT=="sqlite"&&($dn||$re)?$Fa:$l),$re,($L["Comment"]!=$R["Comment"]?$L["Comment"]:null),($L["Engine"]&&$L["Engine"]!=$R["Engine"]?$L["Engine"]:""),($L["Collation"]&&$L["Collation"]!=$R["Collation"]?$L["Collation"]:""),($L["Auto_increment"]!=""?number($L["Auto_increment"]):""),$bj);if($J&&!Queries::$queries)redirect($y);queries_redirect($y,$nh,$J);}}if($a!="")page_header(lang(36).": ".h($a),["table"=>$a,lang(36)]);else
page_header(lang(78),[lang(78)]);if(!$_POST){$Mm=Driver::get()->getTypes();$L=["Engine"=>$_COOKIE["neo_engine"],"fields"=>[["field"=>"","type"=>(isset($Mm["int"])?"int":(isset($Mm["integer"])?"integer":"")),"on_update"=>""]],"partition_names"=>[""],];if($a!=""){$L=$R;$L["name"]=$a;$L["fields"]=[];if(!$_GET["auto_increment"])$L["Auto_increment"]="";foreach($Ii
as$k){$k["generated"]=$k["generated"]?:(isset($k["default"])?"DEFAULT":"");$L["fields"][]=$k;}if($Zi){$L+=$dj;$L["partition_names"][]="";$L["partition_values"][]="";}}}$jg=[];if($L["Collation"])$jg[$L["Collation"]]=true;foreach($L["fields"]as$k){if($k["collation"])$jg[$k["collation"]]=true;}$Fb=Admin::get()->getCollations(array_keys($jg));$zd=Driver::get()->engines();foreach($zd
as$yd){if(!strcasecmp($yd,$L["Engine"])){$L["Engine"]=$yd;break;}}$ah=max_input_vars(12,20);if($ah){$cf=(count($L["fields"])>$ah?"":" hidden");echo"<p".($cf?" id='max-fields' data-columns='$ah'":"")." class='error$cf'>".max_input_vars_error()."\n";}echo"<form action='' method='post' id='form'>\n";if(support("columns")||$a==""){echo"<p>",lang(218),": ","<input class='input' name='name' data-maxlength='64' value='",h($L["name"]),"' autocapitalize='off'",(($a==""&&!$_POST)?" autofocus":""),">";if($zd)echo" ",html_select("Engine",[""=>"(".lang(219).")"]+$zd,$L["Engine"]),help_script_command("value",true);if($Fb&&!preg_match("~sqlite|mssql~",DIALECT))echo" ",html_select("Collation",[""=>"(".lang(92).")"]+$Fb,$L["Collation"]);echo" <input type='submit' class='button default' value='",lang(116),"'>","</p>";}if(support("columns")&&($a==""||!Driver::get()->isPartition($a))){echo"<div class='scrollable'>\n","<table id='edit-fields' class='nowrap'>\n";edit_fields($L["fields"],$Fb,"TABLE",$te);echo"</table>\n",script("initFieldsEditing(gid('edit-fields'));");if(support("move_col"))echo
script("initSortable('#edit-fields tbody');");echo"</div>\n","<p>",lang(49),": ","<input type='number' class='input size' name='Auto_increment' size='6' value='",h($L["Auto_increment"]),"'>";$Rb=$_POST?$_POST["comments"]:Admin::get()->getSettings()->getParameter("commentsOpened");$Pb=$Rb?"":"hidden";if(support("comment")){echo
checkbox("comments",1,$Rb,lang(48),"editingCommentsClick(this);","jsonly")," ";if(preg_match('~\n~',$L["Comment"]))echo"<textarea name='Comment' rows='2' cols='20'",($Pb?" class='$Pb'":""),">",h($L["Comment"]),"</textarea>";else
echo"<input name='Comment' value='",h($L["Comment"]),"' data-maxlength='",(Connection::get()->isMinVersion("5.5")?2048:60),"' class='input $Pb'>";}echo"</p>\n<p>","<input type='submit' class='button default' value='",lang(116),"'>";}elseif($a!="")echo"<p>";if($a!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(lang(220,$a)),"</p>\n";if($Zi&&(DIALECT=="sql"||$a=="")){echo"<div class='field-sets'>\n";$aj=preg_match('~RANGE|LIST~',$L["partition_by"]);print_fieldset_start("partition",lang(221),"split",(bool)$L["partition_by"]);echo"<p>",html_select("partition_by",array_merge([""],$Zi),$L["partition_by"]),help_script_command("value.replace(/./, 'PARTITION BY \$&')",true),script("qsl('select').onchange = partitionByChange;"),"(<input class='input' name='partition' value='",h($L["partition"]),"'>) ",lang(50),": ","<input type='number' name='partitions' class='input size ",($aj||!$L["partition_by"]?"hidden":""),"' value='",h($L["partitions"]),"'>","</p>\n","<table id='partition-table'",($aj?"":" class='hidden'"),">\n","<thead><tr><th>",lang(222),"</th><th>",lang(52),"</th></tr></thead>\n";foreach($L["partition_names"]as$u=>$X){echo"<tr>","<td><input class='input' name='partition_names[]' value='",h($X),"' autocapitalize='off'>";if($u==count($L["partition_names"])-1)echo
script("qsl('input').oninput = partitionNameChange;");echo"</td>","<td><input class='input' name='partition_values[]' value='",h(isset($L["partition_values"][$u])?$L["partition_values"][$u]:""),"'></td>","</tr>\n";}echo"</table>\n","</p>\n";print_fieldset_end("partition");echo"</div>\n";}echo
input_token(),"</form>\n";}elseif(isset($_GET["indexes"])){$a=$_GET["indexes"];$zf=["PRIMARY","UNIQUE","INDEX"];$R=table_status1($a,true);$xf=Driver::get()->getIndexAlgorithms($R);$d=Connection::get();$Wg=$d->isMariaDB();if(preg_match('~MyISAM|M?aria'.($d->isMinVersion($Wg?"10.0.5":"5.6")?'|InnoDB':'').'~i',$R["Engine"]))$zf[]="FULLTEXT";if(preg_match('~MyISAM|M?aria'.($d->isMinVersion($Wg?"10.2.2":"5.7")?'|InnoDB':'').'~i',$R["Engine"]))$zf[]="SPATIAL";if($Wg&&$d->isMinVersion("11.7")&&preg_match('~MyISAM|InnoDB~i',$R["Engine"]))$zf[]="VECTOR";$t=indexes($a);$l=fields($a);$Aj=[];if(DIALECT=="mongo"){$Aj=$t["_id_"];unset($zf[0]);unset($t["_id_"]);}$L=$_POST;if($L){$P=Admin::get()->getSettings();if($P->getParameter("indexOptions")!==null)$P->updateParameter("indexOptions",null);}if($_POST&&!$_POST["add"]&&!$_POST["drop_col"]){$Ha=[];foreach($L["indexes"]as$s){$A=$s["name"];if(in_array($s["type"],$zf)){$c=[];$Fg=[];$Mc=[];$ti=[];$wf=$xf?(in_array($s["algorithm"],$xf)?$s["algorithm"]:first($xf)):"";$yf=(support("partial_indexes")?$s["partial"]:"");$Yk=[];ksort($s["columns"]);foreach($s["columns"]as$u=>$b){if($b!=""){$v=isset($s["lengths"][$u])?$s["lengths"][$u]:null;$Kc=isset($s["descs"][$u])?$s["descs"][$u]:null;$si=isset($s["opclasses"][$u])?$s["opclasses"][$u]:null;$Yk[]=($l[$b]?idf_escape($b):$b).($v?"(".(+$v).")":"").($si!=""?" ".idf_escape($si):"").($Kc?" DESC":"");$c[]=$b;$Fg[]=($v?:null);$Mc[]=$Kc;$ti[]="$si";}}$Kd=$t[$A];if($Kd){ksort($Kd["columns"]);ksort($Kd["lengths"]);ksort($Kd["descs"]);if($s["type"]==$Kd["type"]&&array_values($Kd["columns"])===$c&&(!$Kd["lengths"]||array_values($Kd["lengths"])===$Fg)&&array_values($Kd["descs"])===$Mc&&(!$Kd["opclasses"]||array_values($Kd["opclasses"])===$ti)&&(!$xf||$Kd["algorithm"]===$wf)&&$Kd["partial"]==$yf){unset($t[$A]);continue;}}if($c)$Ha[]=[$s["type"],$A,$Yk,$wf,$yf];}}foreach($t
as$A=>$Kd)$Ha[]=[$Kd["type"],$A,"DROP"];if(!$Ha)redirect(ME."table=".urlencode($a));queries_redirect(ME."table=".urlencode($a),lang(223),alter_indexes($a,$Ha));}page_header(lang(176).": ".h($a),["table"=>$a,lang(176)]);$ee=array_keys($l);if($_POST["add"]){foreach($L["indexes"]as$u=>$s){if($s["columns"][count($s["columns"])]!="")$L["indexes"][$u]["columns"][]="";}$s=end($L["indexes"]);if($s["type"]||array_filter($s["columns"],'strlen'))$L["indexes"][]=["columns"=>[1=>""]];}if(!$L){foreach($t
as$u=>$s){$t[$u]["name"]=$u;$t[$u]["columns"][]="";}$t[]=["columns"=>[1=>""]];$L["indexes"]=$t;}$Fg=(DIALECT=="sql"||DIALECT=="mssql");$ti=Driver::get()->getIndexOpclasses();if($_POST)$cl=$_POST["options"];else{$cl=false;foreach($t
as$s){if(array_filter(isset($s["lengths"])?$s["lengths"]:[])||array_filter(isset($s["descs"])?$s["descs"]:[])||array_filter(isset($s["opclasses"])?$s["opclasses"]:[])||(isset($s["partial"])?$s["partial"]:"")!=""){$cl=true;break;}}}echo"<form action='' method='post'>\n","<div class='scrollable'>\n","<table class='nowrap'>\n","<thead><tr>","<th id='label-type'>",lang(224),"</th>";$yi="class='idxopts".($cl?"":" hidden")."'";if(count($xf)>1)echo"<th id='label-method' $yi>",lang(225),doc_link(['sql'=>'create-index.html#create-index-storage-engine-index-types','mariadb'=>'ha-and-performance/optimization-and-tuning/optimization-and-indexes/storage-engine-index-types',]),"</th>";echo"<th><input type='submit' hidden>",lang(53).($Fg?"<span ".str_replace("class='","class='note ",$yi)."> (".lang(54).")</span>":"");if($Fg||support("descidx"))echo
checkbox("options",1,$cl,lang(98),"indexOptionsShow(this.checked)","jsonly")."\n";echo"</th>","<th id='label-name'>",lang(226),"</th>";if(support("partial_indexes"))echo"<th id='label-condition' $yi>",lang(55),"</th>";echo"<th>","<button name='add[0]' value='1' title='",lang(100),"' class='button light hidden'>",icon_solo("add"),"</button>","</th>","</tr></thead>\n";if($Aj){echo"<tr><td>PRIMARY<td>";foreach($Aj["columns"]as$b)echo
select_input(" disabled",array_combine($ee,$ee),$b),"<label><input type='checkbox' disabled>".lang(63)."</label> ";echo"<td><td>\n";}$eg=1;foreach($L["indexes"]as$s){if(!$_POST["drop_col"]||$eg!=key($_POST["drop_col"])){echo"<tr><td>",html_select("indexes[$eg][type]",[-1=>""]+$zf,$s["type"],($eg==count($L["indexes"])?"indexesAddRow.call(this);":""),"label-type"),"</td>";if(count($xf)>1)echo"<td $yi>",html_select("indexes[$eg][algorithm]",array_merge([""],$xf),$s['algorithm'],"label-method"),"</td>";echo"<td>";ksort($s["columns"]);$q=1;foreach($s["columns"]as$u=>$b){echo"<span>".select_input(" name='indexes[$eg][columns][$q]' title='".lang(44)."'",($l&&($b==""||$l[$b])?array_combine($ee,$ee):[]),$b,"partial(indexesChangeColumn, '".js_escape(DIALECT=="sql"?"":$_GET["indexes"]."_")."')"),"<span $yi>";if($Fg)echo"<input type='number' name='indexes[$eg][lengths][$q]' class='input size' value='".(h(isset($s["lengths"][$u])?$s["lengths"][$u]:"")),"' title='".lang(97),"'>";if($ti){$si=isset($s["opclasses"][$u])?$s["opclasses"][$u]:"";echo
html_select("indexes[$eg][opclasses][$q]",[""=>"(".lang(227).")"]+array_combine($ti,$ti)+($si!=""?[$si=>$si]:[]),$si),'';}if(support("descidx"))echo
checkbox("indexes[$eg][descs][$q]",1,isset($s["descs"][$u])?$s["descs"][$u]:false,lang(63));echo"<br></span></span>";$q++;}echo"</td>","<td><input name='indexes[$eg][name]' value='",h($s["name"]),"' class='input' autocapitalize='off' aria-labelledby='label-name'></td>\n";if(support("partial_indexes"))echo"<td $yi><input name='indexes[$eg][partial]' value='".h($s["partial"])."' autocapitalize='off' aria-labelledby='label-condition'>\n";echo"<td>","<button name='drop_col[$eg]' value='1' title='",lang(59),"' class='button light'>",icon_solo("remove"),"</button>",script("qsl('button').onclick = onRemoveIndexRowClick;"),"</td>\n";}$eg++;}echo"</table>\n","</div>\n","<p>","<input type='submit' class='button default' value='",lang(116),"'>",input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["database"])){$L=$_POST;if($_POST&&!isset($_POST["add_x"])){$A=trim($L["name"]);if($_POST["drop"]){$_GET["db"]="";queries_redirect(remove_from_uri("db|database"),lang(228),drop_databases([DB]));}elseif(DB!==$A){if(DB!=""){$_GET["db"]=$A;queries_redirect(preg_replace('~\bdb=[^&]*&~','',ME)."db=".urlencode($A),lang(229),rename_database($A,$L["collation"]));}else{$g=explode("\n",str_replace("\r","",$A));$El=true;$xg="";foreach($g
as$h){if(count($g)==1||$h!=""){if(!create_database($h,$L["collation"]))$El=false;$xg=$h;}}restart_session();set_session("dbs",null);queries_redirect(ME."db=".urlencode($xg),lang(230),$El);}}else{if(!$L["collation"])redirect(substr(ME,0,-1));query_redirect("ALTER DATABASE ".idf_escape($A).(preg_match('~^[a-z0-9_]+$~i',$L["collation"])?" COLLATE $L[collation]":""),substr(ME,0,-1),lang(231));}}if(DB!="")page_header(lang(70).": ".h(DB),[lang(70)]);else
page_header(lang(76),[lang(76)]);$A=DB;if($_POST)$A=$L["name"];elseif(DB!="")$L["collation"]=db_collation(DB,collations());elseif(DIALECT=="sql"){foreach(get_vals("SHOW GRANTS")as$Ke){if(preg_match('~ ON (`(([^\\\\`]|``|\\\\.)*)%`\.\*)?~',$Ke,$z)&&$z[1]){$A=stripcslashes(idf_unescape("`$z[2]`"));break;}}}$Fb=Admin::get()->getCollations($L["collation"]?[$L["collation"]]:[]);echo"<form action='' method='post'>\n","<p>";if($_POST["add_x"]||strpos($A,"\n"))echo"<textarea id='name' name='name' rows='10' cols='40'>",h($A),"</textarea><br>\n";else
echo"<input class='input' name='name' id='name' value='",h($A),"' data-maxlength='64' autocapitalize='off' autofocus>\n";if($Fb)echo
html_select("collation",[""=>"(".lang(92).")"]+$Fb,$L["collation"]),doc_link(['sql'=>"charset-charsets.html",'mariadb'=>"reference/data-types/string-data-types/character-sets/supported-character-sets-and-collations",]),"\n";echo"<input type='submit' class='button default' value='",lang(116),"'>\n";if(DB!="")echo"<input type='submit' class='button' name='drop' value='".lang(167)."'>".confirm(lang(220,DB))."\n";elseif(!$_POST["add_x"]&&$_GET["db"]=="")echo"<button name='add_x' value='1' title='",lang(100),"' class='button light'>",icon_solo("add"),"</button>\n";echo
input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["call"])){$oa=$_GET["name"]?:$_GET["call"];page_header(lang(232).": ".h($oa),[lang(232)]);$ok=routine($_GET["call"],(isset($_GET["callf"])?"FUNCTION":"PROCEDURE"));$tf=[];$Ni=[];foreach($ok["fields"]as$q=>$k){if(substr($k["inout"],-3)=="OUT"&&DIALECT=='sql')$Ni[$q]="@".idf_escape($k["field"])." AS ".idf_escape($k["field"]);if(!$k["inout"]||substr($k["inout"],0,2)=="IN")$tf[]=$q;}if($_POST){$mb=[];foreach($ok["fields"]as$u=>$k){$X="";if(in_array($u,$tf)){$X=process_input($k);if($X===false)$X="''";if(isset($Ni[$u]))Connection::get()->query("SET @".idf_escape($k["field"])." = $X");}if(isset($Ni[$u]))$mb[]="@".idf_escape($k["field"]);elseif(in_array($u,$tf))$mb[]=$X;}$I=(isset($_GET["callf"])?"SELECT ":"CALL ").($ok["returns"]&&$ok["returns"]["type"]=="record"?"* FROM ":"").table($oa)."(".implode(", ",$mb).")";$tl=microtime(true);$J=Connection::get()->multiQuery($I);$za=Connection::get()->getAffectedRows();echo
Admin::get()->formatSelectQuery($I,$tl,!$J);if(!$J)echo"<p class='error'>".error()."\n";else{$Yb=connect();if($Yb)$Yb->selectDatabase(DB);$Th=false;do{$J=Connection::get()->storeResult();if($Th&&Connection::get()->getError())echo"<p class='error'>".error()."\n";elseif(is_object($J))print_select_result($J,$Yb);else
echo"<p class='message'>".lang(233,$za)." <span class='time'>".@date("H:i:s")."</span>\n";}while($Th=Connection::get()->nextResult());if($Ni)print_select_result(Connection::get()->query("SELECT ".implode(", ",$Ni)));}}echo"<form action='' method='post'>\n";if($tf){echo"<table class='box'>\n";foreach($tf
as$u){$k=$ok["fields"][$u];$A=$k["field"];echo"<tr><th>".Admin::get()->getFieldName($k);$Y=isset($_POST["fields"][$A])?$_POST["fields"][$A]:"";if($Y!=""){if($k["type"]=="set")$Y=implode(",",$Y);}input($k,$Y,(string)(isset($_POST["function"][$A])?$_POST["function"][$A]:""));echo"\n";}echo"</table>\n";}echo"<p>\n","<input type='submit' class='button' value='",lang(232),"'>\n",input_token(),"</p>\n","</form>\n";$Ob=$ok["comment"];if($Ob!==null&&$Ob!==""){$Ob=h(trim($ok["comment"],"\n"));if(preg_match('~^ +~',$Ob,$_)){preg_match_all("~^($_[0]|$)~m",$Ob,$Kg);if(count($Kg[0])==substr_count($Ob,"\n"))$Ob=preg_replace("~^($_[0])~m","",$Ob);}$Ob=preg_replace('~(^|[^\n]\n)(Description|Parameters|Example)\n~',"$1\n<strong>$2</strong>\n",$Ob);echo"<pre class='comment'>$Ob</pre>\n";}}elseif(isset($_GET["foreign"])){$a=$_GET["foreign"];$A=$_GET["name"];$L=$_POST;if($_POST&&!$_POST["add"]&&!$_POST["change"]&&!$_POST["change-js"]){if(!$_POST["drop"]){$L["source"]=array_filter($L["source"],'strlen');ksort($L["source"]);$gm=[];foreach($L["source"]as$u=>$X)$gm[$u]=$L["target"][$u];$L["target"]=$gm;}if(DIALECT=="sqlite")$J=recreate_table($a,$a,[],[],[" $A"=>($L["drop"]?"":" ".format_foreign_key($L))]);else{$Ha="ALTER TABLE ".table($a);$J=($A==""||queries("$Ha DROP ".(DIALECT=="sql"?"FOREIGN KEY ":"CONSTRAINT ").idf_escape($A)));if(!$L["drop"])$J=queries("$Ha ADD".format_foreign_key($L));}queries_redirect(ME."table=".urlencode($a),($L["drop"]?lang(234):($A!=""?lang(235):lang(236))),(bool)$J);if(!$L["drop"])Admin::get()->addError(lang(237));}if($A!="")page_header(lang(238).": ".h($A),["table"=>$a,lang(238)]);else
page_header(lang(180).": ".h($a),["table"=>$a,lang(180)]);if($_POST){ksort($L["source"]);if($_POST["change"]||$_POST["change-js"])$L["target"]=[];else$L["source"][]="";}elseif($A!=""){$te=foreign_keys($a);$L=$te[$A];$L["source"][]="";}else{$L["table"]=$a;$L["source"]=[""];}echo"<form action='' method='post'>\n";$kl=array_keys(fields($a));if($L["db"]!="")Connection::get()->selectDatabase($L["db"]);if($L["ns"]!=""){$Ji=get_schema();set_schema($L["ns"]);}$Wj=array_keys(array_filter(table_status('',true),'AdminNeo\fk_support'));$gm=$Wj?array_keys(fields(in_array($L["table"],$Wj)?$L["table"]:reset($Wj))):[];$oi="this.form['change-js'].value = '1'; this.form.submit();";echo"<p>","<span id='label-table'>",lang(239),":</span> ",html_select("table",$Wj,$L["table"],$oi,"label-table");if(DIALECT!="sqlite"){$_c=[];foreach(Admin::get()->getDatabases()as$h){if(!information_schema($h))$_c[]=$h;}echo"<span id='label-db'>",lang(240),":</span> ",html_select("db",$_c,$L["db"]!=""?$L["db"]:$_GET["db"],$oi,"label-db");}echo
input_hidden("change-js"),"<noscript><input type='submit' class='button' name='change' value='",lang(241),"'></noscript>","</p>\n","<table>","<thead><tr><th id='label-source'>",lang(177),"<th id='label-target'>",lang(178),"</thead>\n";$eg=0;foreach($L["source"]as$u=>$X){echo"<tr>","<td>".html_select("source[".(+$u)."]",[-1=>""]+$kl,$X,($eg==count($L["source"])-1?"foreignAddRow.call(this);":""),"label-source"),"<td>".html_select("target[".(+$u)."]",$gm,isset($L["target"][$u])?$L["target"][$u]:null,"","label-target");$eg++;}echo"</table>\n","<noscript><p><input type='submit' class='button' name='add' value='",lang(242),"'></p></noscript>","<p>\n","<span id='label-delete'>".lang(94),":</span> ",html_select("on_delete",[-1=>""]+Driver::get()->getOnActions(),$L["on_delete"],"","label-delete"),"<span id='label-update'>".lang(93),":</span> ",html_select("on_update",[-1=>""]+Driver::get()->getOnActions(),$L["on_update"],"","label-update");if(DRIVER=='pgsql')echo
html_select("deferrable",['NOT DEFERRABLE','DEFERRABLE','DEFERRABLE INITIALLY DEFERRED'],$L["deferrable"]);echo
doc_link(['sql'=>"innodb-foreign-key-constraints.html",'mariadb'=>"architecture/server-constraints/foreign-key-constraints",]),"</p>\n<p>","<input type='submit' class='button default' value='",lang(116),"'>";if($A!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(lang(220,$A));echo
input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["view"])){$a=$_GET["view"];$L=$_POST;$Ki="VIEW";if(DIALECT=="pgsql"&&$a!=""){$vl=table_status1($a);$Ki=strtoupper($vl["Engine"]);}if($_POST){$A=trim($L["name"]);$Ma=" AS\n$L[select]";$y=ME."table=".urlencode($A);$nh=lang(243);$U=($_POST["materialized"]?"MATERIALIZED VIEW":"VIEW");if(!$_POST["drop"]&&$a==$A&&DIALECT!="sqlite"&&$U=="VIEW"&&$Ki=="VIEW")query_redirect((DIALECT=="mssql"?"ALTER":"CREATE OR REPLACE")." VIEW ".table($A).$Ma,$y,$nh);else{$im="adminneo_".uniqid();drop_create("DROP $Ki ".table($a),"CREATE $U ".table($A).$Ma,"DROP $U ".table($A),"CREATE $U ".table($im).$Ma,"DROP $U ".table($im),($_POST["drop"]?substr(ME,0,-1):$y),lang(244),$nh,lang(245),$a,$A);}}if(!$_POST&&$a!=""){$L=view($a);$L["name"]=$a;$L["materialized"]=($Ki!="VIEW");if($j=error())Admin::get()->addError($j);}if($a!="")page_header(lang(37).": ".h($a),["table"=>$a,lang(37)]);else
page_header(lang(246),[lang(246)]);echo"<form action='' method='post'>\n","<p>",lang(226),":","<input class='input' name='name' value='",h($L["name"]),"' data-maxlength='64' autocapitalize='off'>\n";if(support("materializedview"))echo
checkbox("materialized",1,$L["materialized"],lang(168));echo"</p>\n<p>";textarea("select",$L["select"]);echo"</p>\n<p>","<input type='submit' class='button default' value='",lang(116),"'>\n";if($a!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>\n",confirm(lang(220,$a));echo
input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["event"])){$ea=$_GET["event"];$Mf=["YEAR","QUARTER","MONTH","DAY","HOUR","MINUTE","WEEK","SECOND","YEAR_MONTH","DAY_HOUR","DAY_MINUTE","DAY_SECOND","HOUR_MINUTE","HOUR_SECOND","MINUTE_SECOND"];$wl=["ENABLED"=>"ENABLE","DISABLED"=>"DISABLE","SLAVESIDE_DISABLED"=>"DISABLE ON SLAVE"];$L=$_POST;if($_POST){if($_POST["drop"])query_redirect("DROP EVENT ".idf_escape($ea),substr(ME,0,-1),lang(247));elseif(in_array($L["INTERVAL_FIELD"],$Mf)&&isset($wl[$L["STATUS"]])){$zk="\nON SCHEDULE ".($L["INTERVAL_VALUE"]?"EVERY ".q($L["INTERVAL_VALUE"])." $L[INTERVAL_FIELD]".($L["STARTS"]?" STARTS ".q($L["STARTS"]):"").($L["ENDS"]?" ENDS ".q($L["ENDS"]):""):"AT ".q($L["STARTS"]))." ON COMPLETION".($L["ON_COMPLETION"]?"":" NOT")." PRESERVE";queries_redirect(substr(ME,0,-1),($ea!=""?lang(248):lang(249)),(bool)queries(($ea!=""?"ALTER EVENT ".idf_escape($ea).$zk.($ea!=$L["EVENT_NAME"]?"\nRENAME TO ".idf_escape($L["EVENT_NAME"]):""):"CREATE EVENT ".idf_escape($L["EVENT_NAME"]).$zk)."\n".$wl[$L["STATUS"]]." COMMENT ".q($L["EVENT_COMMENT"]).rtrim(" DO\n$L[EVENT_DEFINITION]",";").";"));}}if($ea!="")page_header(lang(250).": ".h($ea),[lang(250)]);else
page_header(lang(251),[lang(251)]);if(!$L&&$ea!=""){$M=get_rows("SELECT * FROM information_schema.EVENTS WHERE EVENT_SCHEMA = ".q(DB)." AND EVENT_NAME = ".q($ea));$L=reset($M);}echo"<form action='' method='post'>\n","<table class='box box-light'>\n","<tr><th>",lang(226),"</th><td>","<input class='input' name='EVENT_NAME' value='",h($L["EVENT_NAME"]),"' data-maxlength='64' autocapitalize='off'>","</td></tr>\n","<tr><th title='datetime'>",lang(252),"</th><td>","<input class='input' name='STARTS' value='",h("$L[EXECUTE_AT]$L[STARTS]"),"'>","</td></tr>\n","<tr><th title='datetime'>",lang(253),"</th><td>","<input class='input' name='ENDS' value='",h($L["ENDS"]),"'>","</td></tr>\n","<tr><th>",lang(254),"</th><td>","<input type='number' name='INTERVAL_VALUE' value='",h($L["INTERVAL_VALUE"]),"' class='input size'> ",html_select("INTERVAL_FIELD",$Mf,$L["INTERVAL_FIELD"]),"</td></tr>\n","<tr><th>",lang(159),"</th><td>",html_select("STATUS",$wl,$L["STATUS"]),"</td></tr>\n","<tr><th>",lang(48),"</th><td>","<input class='input' name='EVENT_COMMENT' value='",h($L["EVENT_COMMENT"]),"' data-maxlength='64'>","</td></tr>\n","<tr><th></th><td>",checkbox("ON_COMPLETION","PRESERVE",$L["ON_COMPLETION"]=="PRESERVE",lang(255)),"</td></tr>\n","</table>\n","<p>";textarea("EVENT_DEFINITION",$L["EVENT_DEFINITION"]);echo"</p>\n","<p>","<input type='submit' class='button default' value='",lang(116),"'>";if($ea!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(lang(220,$ea));echo"</p>\n",input_token(),"</form>\n";}elseif(isset($_GET["procedure"])){$oa=($_GET["name"]?:$_GET["procedure"]);$ok=(isset($_GET["function"])?"FUNCTION":"PROCEDURE");$L=$_POST;$L["fields"]=(array)$L["fields"];if($_POST&&!process_fields($L["fields"])){foreach($L["fields"]as$u=>$k){if($k["field"]=="")unset($L["fields"][$u]);}$ji=routine_id($oa,routine($_GET["procedure"],$ok));$Qh=routine_id($L["name"],$L);$e=create_routine($ok,$L);$y=substr(ME,0,-1);$nh=lang(256);if(!$_POST["drop"]&&$ji==$Qh&&(DIALECT!="sql"||Connection::get()->isMariaDB()))query_redirect(substr_replace($e,' OR REPLACE',6,0),$y,$nh);else{$im="adminneo_".uniqid();drop_create("DROP $ok $ji",$e,"DROP $ok $Qh",create_routine($ok,["name"=>$im]+$L),"DROP $ok ".routine_id($im,$L),$y,lang(257),$nh,lang(258),$oa,$L["name"]);}}if($oa!=""){$T=isset($_GET["function"])?lang(259):lang(260);page_header($T.": ".h($oa),[$T]);}else{$T=isset($_GET["function"])?lang(261):lang(262);page_header($T,[$T]);}if(!$_POST){if($oa=="")$L["language"]="sql";else{$L=routine($_GET["procedure"],$ok);$L["name"]=$oa;}}$sb=get_vals("SHOW CHARACTER SET");sort($sb);$pk=routine_languages();echo"<form action='' method='post' id='form'>\n","<p>",lang(226),": ","<input class='input' name='name' value='",h($L["name"]),"' data-maxlength='64' autocapitalize='off'>";if($pk)echo"<span id='label-language'>",lang(10),":</span> ",html_select("language",$pk,$L["language"],"","label-language");echo"<input type='submit' class='button default' value='",lang(116),"'>","</p>\n","<div class='scrollable'>\n","<table class='nowrap' id='edit-fields'>\n";edit_fields($L["fields"],$sb,$ok);if(isset($_GET["function"])){echo"<tbody><tr>";if(support("move_col"))echo"<th></th>";echo"<th>",lang(263),"</th>";edit_type("returns",(array)$L["returns"],$sb,[],(DIALECT=="pgsql"?["void","trigger"]:[]));echo"<td></td>","</tr></tbody>\n";}echo"</table>\n",script("initFieldsEditing(gid('edit-fields'));");if(support("move_col"))echo
script("initSortable('#edit-fields tbody');");echo"</div>\n","<p>";textarea("definition",$L["definition"],20);echo"</p>\n<p>","<input type='submit' class='button default' value='",lang(116),"'>";if($oa!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(lang(220,$oa));echo
input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["check"])){$a=$_GET["check"];$A=$_GET["name"];$L=$_POST;if($L){if(DIALECT=="sqlite")$El=recreate_table($a,$a,[],[],[],"",[],"$A",($L["drop"]?"":$L["clause"]));else{$El=($A==""||queries("ALTER TABLE ".table($a)." DROP CONSTRAINT ".idf_escape($A)));if(!$L["drop"])$El=(bool)queries("ALTER TABLE ".table($a)." ADD".($L["name"]!=""?" CONSTRAINT ".idf_escape($L["name"]):"")." CHECK ($L[clause])");}queries_redirect(ME."table=".urlencode($a),($L["drop"]?lang(264):($A!=""?lang(265):lang(266))),$El);}if($A!="")page_header(lang(267).": ".h($A),["table"=>$a,lang(267)]);else
page_header(lang(182).": ".h($a),["table"=>$a,lang(182)]);if(!$L){$xb=Driver::get()->checkConstraints($a);$L=["name"=>$A,"clause"=>$xb[$A]];}echo"<form action='' method='post'>\n","<p>";if(DIALECT!="sqlite")echo
lang(226).': <input name="name" value="'.h($L["name"]).'" class="input" data-maxlength="64" autocapitalize="off"> ';echo
doc_link(['sql'=>"create-table-check-constraints.html",'mariadb'=>"reference/sql-statements/data-definition/constraint",],"?"),"</p>\n<p>";textarea("clause",$L["clause"]);echo"</p>\n<p>","<input type='submit' class='button default' value='",lang(116),"'>";if($A!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(lang(220,$A));echo
input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["trigger"])){$a=$_GET["trigger"];$A=isset($_GET["name"])?$_GET["name"]:"";$Fm=trigger_options();$L=trigger($A,$a)+["Trigger"=>$a."_bi"];if($_POST){if(in_array($_POST["Timing"],$Fm["Timing"])&&in_array($_POST["Event"],$Fm["Event"])&&in_array($_POST["Type"],$Fm["Type"])){$mi=" ON ".table($a);$fd="DROP TRIGGER ".idf_escape($A).(DIALECT=="pgsql"?$mi:"");$y=ME."table=".urlencode($a);if($_POST["drop"])query_redirect($fd,$y,lang(268));else{if($A!="")queries($fd);queries_redirect($y,($A!=""?lang(269):lang(270)),(bool)queries(create_trigger($mi,$_POST)));if($A!="")queries(create_trigger($mi,$L+["Type"=>reset($Fm["Type"])]));}}$L=$_POST;}if($A!="")page_header(lang(271).": ".h($A),["table"=>$a,lang(271)]);else
page_header(lang(184).": ".h($a),["table"=>$a,lang(184)]);echo"<form action='' method='post' id='form'>\n","<table class='box box-light'>\n","<tr><th id='label-time'>",lang(272),"</th><td>",html_select("Timing",$Fm["Timing"],$L["Timing"],"triggerChange(/^".js_escape_re($a)."_[ba][iud]$/, '".js_escape($a)."', this.form);","label-time"),"</td></tr>\n","<tr><th id='label-event'>",lang(273),"</th><td>",html_select("Event",$Fm["Event"],$L["Event"],"this.form['Timing'].onchange();","label-event");if(in_array("UPDATE OF",$Fm["Event"]))echo" <input name='Of' value='".h($L["Of"])."' class='input hidden'>";echo"</td></tr>\n","<tr><th id='label-type'>",lang(45),"</th><td>",html_select("Type",$Fm["Type"],$L["Type"],"","label-type"),"</td></tr>\n","</table>\n","<p>",lang(226),"<input class='input' name='Trigger' value='",h($L["Trigger"]),"' data-maxlength='64' autocapitalize='off'>","</p>\n",script("gid('form')['Timing'].onchange();"),"<p>";textarea("Statement",$L["Statement"]);echo"</p>\n","<p>","<input type='submit' class='button default' value='",lang(116),"'>";if($A!="")echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>",confirm(lang(220,$A));echo"</p>\n",input_token(),"</form>\n";}elseif(isset($_GET["user"])){$qa=$_GET["user"];$Ej=[""=>["All privileges"=>""]];foreach(get_rows("SHOW PRIVILEGES")as$L){foreach(explode(",",($L["Privilege"]=="Grant option"?"":$L["Context"]))as$cc)$Ej[$cc=="File access on server"?"Server Admin":$cc][$L["Privilege"]]=$L["Comment"];}unset($Ej["Server Admin"]["Usage"]);foreach($Ej["Tables"]as$u=>$X)unset($Ej["Databases"][$u]);$Ph=[];if($_POST){foreach($_POST["objects"]as$u=>$X)$Ph[$X]=(array)$Ph[$X]+(array)$_POST["grants"][$u];}$Me=[];if(isset($_GET["host"])&&($J=Connection::get()->query("SHOW GRANTS FOR ".q($qa)."@".q($_GET["host"])))){while($L=$J->fetchRow()){if(preg_match('~GRANT (.*) ON (.*) TO ~',$L[0],$z)&&preg_match_all('~ *([^(,]*[^ ,(])( *\([^)]+\))?~',$z[1],$_,PREG_SET_ORDER)){foreach($_
as$X){if($X[1]!="USAGE")$Me["$z[2]$X[2]"][$X[1]]=true;if(preg_match('~ WITH GRANT OPTION~',$L[0]))$Me["$z[2]$X[2]"]["GRANT OPTION"]=true;}}}}$mj=!Connection::get()->isMariaDB()&&Connection::get()->isMinVersion("8");if($_POST){$li=(isset($_GET["host"])?q($qa)."@".q($_GET["host"]):"''");if($_POST["drop"])query_redirect("DROP USER $li",ME."privileges=",lang(274));else{$Sh=q($_POST["user"])."@".q($_POST["host"]);$fj=$_POST["pass"];$mc=false;$J=true;if($li!=$Sh){$mc=(bool)queries("CREATE USER $Sh IDENTIFIED BY ".($_POST["hashed"]?"PASSWORD ":"").q($fj));$J=$mc;}elseif($fj!="")$J=(bool)queries("SET PASSWORD FOR $Sh = ".($mj||$_POST["hashed"]?q($fj):"PASSWORD(".q($fj).")"));if($J){$lk=[];foreach($Ph
as$ci=>$Ke){if(isset($_GET["grant"]))$Ke=array_filter($Ke);$Ke=array_keys($Ke);if(isset($_GET["grant"]))$lk=array_diff(array_keys(array_filter($Ph[$ci],'strlen')),$Ke);elseif($li==$Sh){$ii=array_keys((array)$Me[$ci]);$lk=array_diff($ii,$Ke);$Ke=array_diff($Ke,$ii);unset($Me[$ci]);}if(preg_match('~^(.+)\s*(\(.*\))?$~U',$ci,$z)&&(!grant(false,$lk,$z[2],$z[1],$Sh)||!grant(true,$Ke,$z[2],$z[1],$Sh))){$J=false;break;}}}if($J&&isset($_GET["host"])){if($li!=$Sh)queries("DROP USER $li");elseif(!isset($_GET["grant"])){foreach($Me
as$ci=>$lk){if(preg_match('~^(.+)(\(.*\))?$~U',$ci,$z))grant(false,array_keys($lk),$z[2],$z[1],$Sh);}}}if($J&&!Queries::$queries)redirect(ME."privileges=");queries_redirect(ME."privileges=",(isset($_GET["host"])?lang(275):lang(276)),$J);if($mc)Connection::get()->query("DROP USER $Sh");}}$T=isset($_GET["host"])?lang(6).": ".h("$qa@$_GET[host]"):lang(192);$tm=isset($_GET["host"])?h($qa):lang(192);page_header($T,["privileges"=>['',lang(73)],$tm]);if($_POST){$L=$_POST;$Me=$Ph;}else{$L=$_GET+["host"=>Connection::get()->getValue("SELECT SUBSTRING_INDEX(CURRENT_USER, '@', -1)")];if($Me)$Me[".*"]=[];elseif(DB!="")$Me[idf_escape(addcslashes(DB,"%_\\")).".*"]=[];else$Me["*.* "]=[];}echo"<form action='' method='post'>\n","<table class='box box-light'>\n","<tr><th>",lang(5),"</th>","<td><input class='input' name='host' data-maxlength='60' value='",h($L["host"]),"' autocapitalize='off'></td>\n","<tr><th>",lang(6),"</th>","<td><input class='input' name='user' data-maxlength='80' value='",h($L["user"]),"' autocapitalize='off'></td>\n",'<tr><th>',lang(30),"</th>","<td><input class='input' name='pass' id='pass' value='",h($L["pass"]),"' autocomplete='new-password'>";if(!$mj)echo
checkbox("hashed",1,$L["hashed"],lang(277),"typePassword(this.form['pass'], this.checked);");echo"</td>\n";if(!$L["hashed"])echo
script("typePassword(gid('pass'));");echo"</table>\n","<div class='scrollable'><table class='checkable'>\n","<thead><tr><th colspan='2'>".lang(73).doc_link(['sql'=>"grant.html#priv_level","mariadb"=>"reference/sql-statements/account-management-sql-statements/grant#privilege-levels"])."</th>";$q=0;foreach($Me
as$ci=>$Ke){echo"<th>";if($ci=="*.*")echo"*.*",input_hidden("objects[$q]","*.*");else
echo"<input class='input' name='objects[$q]' value='".h(trim($ci))."' size='10' autocapitalize='off'>";echo"</th>";$q++;}echo"</tr></thead>\n";foreach([""=>"","Server Admin"=>lang(5),"Databases"=>lang(31),"Tables"=>lang(9),"Procedures"=>lang(278),]as$cc=>$Kc){foreach((array)$Ej[$cc]as$Dj=>$Ob){echo"<tr>";if($Kc)echo"<td>$Kc</td>";echo"<td".(!$Kc?" colspan='2'":"").' lang="en" title="'.h($Ob).'">'.h($Dj)."</td>";$q=0;foreach($Me
as$ci=>$Ke){$A="'grants[$q][".h(strtoupper($Dj))."]'";$Y=$Ke[strtoupper($Dj)];$Ij=strpos($ci,"@")!==false;$Oh=$ci==".*";$Da=$Dj=="All privileges";$Le=$Dj=="Grant option";if($ci=="*.*"&&$Dj=="Proxy")echo"<td></td>";elseif($Ij&&$Dj!="Proxy"&&!$Le)echo"<td></td>";elseif($cc=="Server Admin"&&$ci!=(isset($Me["*.*"])?"*.*":".*")&&!(($Ij||$Oh)&&$Dj=="Proxy"))echo"<td></td>";elseif(isset($_GET["grant"]))echo"<td><select name=$A>"."<option></option>"."<option value='1'".($Y?" selected":"").">".lang(279)."</option>"."<option value='0'".($Y=="0"?" selected":"").">".lang(280)."</option>"."</select></td>";else{echo"<td class='center'><label class='block'>","<input type='checkbox' name=$A value='1'".($Y?" checked":"").($Da?" id='grants-$q-all'":(!$Le?" class='grants-$q'":"")).">";if($Da)echo
script("qsl('input').onclick = function () { if (this.checked) formUncheckAll('.grants-$q'); };");elseif(!$Le)echo
script("qsl('input').onclick = function () { if (this.checked) formUncheck('grants-$q-all'); };");echo"</label>";}$q++;}echo"</tr>";}}echo"</table></div>\n","<p>","<input type='submit' class='button default' value='",lang(116),"'>\n";if(isset($_GET["host"]))echo"<input type='submit' class='button' name='drop' value='",lang(167),"'>\n",confirm(lang(220,"$qa@$_GET[host]"));echo
input_token(),"</p>\n","</form>\n";}elseif(isset($_GET["processlist"])){if(support("kill")){if($_POST){$pg=0;foreach((array)$_POST["kill"]as$X){if(kill_process($X))$pg++;}queries_redirect(ME."processlist=",lang(281,$pg),$pg||!$_POST["kill"]);}}page_header(lang(157),[lang(157)]);echo"<form action='' method='post'>\n","<div class='scrollable'>\n","<table class='nowrap checkable'>\n";$q=-1;foreach(process_list()as$q=>$L){if(!$q){echo"<thead><tr lang='en'>".(support("kill")?"<th>":"");foreach($L
as$u=>$X)echo"<th>$u".doc_link(['sql'=>"show-processlist.html#processlist_".strtolower($u),'mariadb'=>"reference/sql-statements/administrative-sql-statements/show/show-processlist",]);echo"</thead>\n","<tbody>\n";}echo"<tr>".(support("kill")?"<td>".checkbox("kill[]",$L[DIALECT=="sql"?"Id":"pid"],0):"");foreach($L
as$u=>$X)echo"<td>".($X!=""&&((DIALECT=="sql"&&$u=="Info"&&preg_match("~Query|Killed~",$L["Command"]))||(DIALECT=="pgsql"&&$u=="query")||(DIALECT=="oracle"&&$u=="sql_text"))?"<code class='jush-".DIALECT."'>".truncate_utf8($X,100).'</code> <a href="'.h(ME.($L["db"]!=""?"db=".urlencode($L["db"])."&":"")."sql=".urlencode($X)).'">'.icon("edit").lang(282).'</a>':h($X));echo"\n";}if($q>=0)echo"</tbody>\n",script("mixin(qsl('tbody'), {onclick: tableClick, ondblclick: event => tableClick(event, true)});");echo"</table>\n","</div>\n","<p>";if(support("kill"))echo($q+1)."/".lang(283,max_connections()),"<p><input type='submit' class='button' value='".lang(284)."'>\n";echo
input_token(),"</p>\n","</form>\n",script("tableCheck();");}elseif(isset($_GET["select"])){$a=$_GET["select"];$R=table_status1($a);$t=indexes($a);$l=fields($a);$te=column_foreign_keys($a);$ei=$R["Oid"];$mk=[];$c=[];$Dk=[];$Ai=[];$mm=null;foreach($l
as$u=>$k){$A=Admin::get()->getFieldName($k);$Jh=html_entity_decode(strip_tags($A),ENT_QUOTES);if(isset($k["privileges"]["select"])&&$A!=""){$c[$u]=$Jh;if(is_shortable($k))$mm=Admin::get()->processSelectionLength();}if(isset($k["privileges"]["where"])&&$A!="")$Dk[$u]=$Jh;if(isset($k["privileges"]["order"])&&$A!="")$Ai[$u]=$Jh;$mk+=$k["privileges"];}list($N,$Ne)=Admin::get()->processSelectionColumns($c,$t);$N=array_unique($N);$Ne=array_unique($Ne);$Wf=count($Ne)<count($N);$Z=Admin::get()->processSelectionSearch($l,$t);$D=Admin::get()->processSelectionOrder($l,$t);$w=Admin::get()->processSelectionLimit();$Ba=$N;if(DIALECT=="pgsql"||DIALECT=="mssql"||DIALECT=="clickhouse"){foreach((array)$_GET["columns"]as$u=>$X){if(isset($Ba[$u])&&$X["fun"])$Ba[$u].=" AS ".idf_escape(apply_sql_function($X["fun"],($X["col"]!=""?$X["col"]:"*")));}}if($_GET["modify"]&&!Admin::get()->isDataEditAllowed())redirect(ME."select=".urlencode($a));if($_GET["val"]&&is_ajax()){header("Content-Type: text/plain; charset=utf-8");foreach($_GET["val"]as$Rm=>$L){$Ma=convert_field($l[key($L)]);$N=[$Ma?:idf_escape(key($L))];$Z[]=where_check($Rm,$l);$K=Driver::get()->select($a,$N,$Z,$N);if(is_object($K))echo
first($K->fetchRow());}exit;}$Aj=$Um=[];foreach($t
as$s){if($s["type"]=="PRIMARY"){$Aj=array_flip($s["columns"]);$Um=($N?$Aj:[]);foreach($Um
as$u=>$X){if(in_array(idf_escape($u),$N))unset($Um[$u]);}break;}}if($ei&&!$Aj){$Aj=$Um=[$ei=>0];$t[]=["type"=>"PRIMARY","columns"=>[$ei]];}$P=Admin::get()->getSettings();if($_POST){$Fn=$Z;if(!$_POST["all"]&&is_array($_POST["check"])){$xb=[];foreach($_POST["check"]as$tb)$xb[]=where_check($tb,$l);$Fn[]="((".implode(") OR (",$xb)."))";}$Fn=($Fn?"\nWHERE ".implode(" AND ",$Fn):"");if($_POST["export"]){$P->updateParameters(["exportFormat"=>$_POST["format"],"exportOutput"=>$_POST["output"],]);dump_headers($a);Admin::get()->dumpTable($a,"");$Be=($N?implode(", ",$Ba):"*").convert_fields($c,$l,$N)."\nFROM ".table($a);$Qe=($Ne&&$Wf?"\nGROUP BY ".implode(", ",$Ne):"").($D?"\nORDER BY ".implode(", ",$D):"");if(!is_array($_POST["check"])||$Aj)$I="SELECT $Be$Fn$Qe";else{$Om=[];foreach($_POST["check"]as$X)$Om[]="(SELECT".limit($Be,"\nWHERE ".($Z?implode(" AND ",$Z)." AND ":"").where_check($X,$l).$Qe,1).")";$I=implode(" UNION ALL ",$Om);}Admin::get()->dumpData($a,"table",$I);exit;}if($_POST["save"]||$_POST["delete"]){$J=true;$za=0;$Yk=[];if(!$_POST["delete"]){$Mk=array_keys($_POST["fields"]+$_POST["function"]);foreach($Mk
as$A){$X=process_input($l[$A]);if($X!==null&&($_POST["clone"]||$X!==false))$Yk[idf_escape($A)]=($X!==false?$X:idf_escape($A));}}if($_POST["delete"]||$Yk){if($_POST["clone"])$I="INTO ".table($a)." (".implode(", ",array_keys($Yk)).")\nSELECT ".implode(", ",$Yk)."\nFROM ".table($a);if($_POST["all"]||($Aj&&is_array($_POST["check"]))||$Wf){$J=($_POST["delete"]?Driver::get()->delete($a,$Fn):($_POST["clone"]?queries("INSERT $I$Fn".Driver::get()->getInsertReturningSql($a)):Driver::get()->update($a,$Yk,$Fn)));$za=Connection::get()->getAffectedRows();if(is_object($J))$za+=$J->getRowsCount();}else{foreach((array)$_POST["check"]as$X){$En="\nWHERE ".($Z?implode(" AND ",$Z)." AND ":"").where_check($X,$l);$J=($_POST["delete"]?Driver::get()->delete($a,$En,1):($_POST["clone"]?queries("INSERT".limit1($a,$I,$En)):Driver::get()->update($a,$Yk,$En,1)));if(!$J)break;$za+=Connection::get()->getAffectedRows();}}}$nh=lang(285,$za);if($_POST["clone"]&&$J&&$za==1){$yg=last_id($J);if($yg)$nh=lang(214," $yg");}queries_redirect(remove_from_uri($_POST["all"]&&$_POST["delete"]?"page":""),$nh,(bool)$J);if(!$_POST["delete"]){$rd=array_filter($l,function($k){return!(isset($k["generated"])?$k["generated"]:null);});edit_form($a,$rd,(array)$_POST["fields"],!$_POST["clone"]);page_footer();exit;}}elseif(!$_POST["import"]){if(!$_POST["val"])Admin::get()->addError(lang(286));else{$El=true;$za=0;foreach($_POST["val"]as$Rm=>$L){$Yk=[];foreach($L
as$u=>$X){$u=bracket_escape($u,true);if(isset($l[$u]["full_type"]))Admin::get()->detectJson($l[$u]["full_type"],$X,false);$Yk[idf_escape($u)]=(preg_match('~char|text~',$l[$u]["type"])||$X!=""?Admin::get()->processFieldInput($l[$u],$X):"NULL");}$El=(bool)Driver::get()->update($a,$Yk," WHERE ".($Z?implode(" AND ",$Z)." AND ":"").where_check($Rm,$l),($Wf||$Aj?0:1)," ");if(!$El)break;$za+=Connection::get()->getAffectedRows();}queries_redirect(remove_from_uri(),lang(285,$za),$El);}}elseif(!is_string($m=get_file("csv_file",true)))Admin::get()->addError(upload_error($m));elseif(!preg_match('~~u',$m))Admin::get()->addError(lang(287));else{$P->updateParameter("exportFormat",$_POST["import_format"]);$Jb=array_keys($l);preg_match_all('~(?>"[^"]*"|[^"\r\n]+)+~',$m,$_);$za=count($_[0]);Driver::get()->begin();$Nk=($_POST["import_format"]=="csv;"?";":($_POST["import_format"]=="tsv"?"\t":","));$M=[];foreach($_[0]as$u=>$X){preg_match_all("~((?>\"[^\"]*\")+|[^$Nk]*)$Nk~",$X.$Nk,$Yg);if(!$u&&!array_diff($Yg[1],$Jb)){$Jb=$Yg[1];$za--;}else{$Yk=[];foreach($Yg[1]as$q=>$Cb)$Yk[idf_escape($Jb[$q])]=($Cb==""&&$l[$Jb[$q]]["null"]?"NULL":q(preg_match('~^".*"$~s',$Cb)?str_replace('""','"',substr($Cb,1,-1)):$Cb));$M[]=$Yk;}}$El=!$M||Driver::get()->insertUpdate($a,$M,$Aj);if($El)Driver::get()->commit();queries_redirect(remove_from_uri("page"),lang(288,$za),$El);Driver::get()->rollback();}}$Ul=Admin::get()->getTableName($R);if(is_ajax()){page_headers();ob_start();}else
page_header(lang(56).": $Ul",[$Ul]);$Hf=null;if(isset($mk["insert"])||!support("table")){$Hf=[];foreach((array)$_GET["where"]as$X){if(isset($te[$X["col"]])&&count($te[$X["col"]])==1&&($X["op"]=="="||(!$X["op"]&&(is_array($X["val"])||!preg_match('~[_%]~',$X["val"])))))$Hf["preset"."[".bracket_escape($X["col"])."]"]=$X["val"];}}Admin::get()->printTableMenu($R,$Hf);if(!$c&&support("table"))echo"<p class='error'>".lang(289).($l||error()==""?".":": ".error())."\n";else{echo"<form id='form' action=''>\n","<div hidden>";hidden_fields_get();if(DB!=""){echo
input_hidden("db",DB);if(isset($_GET["ns"]))echo
input_hidden("ns",$_GET["ns"]);}echo
input_hidden("select",$a),"<input type='submit' class='button' value='".lang(56)."'>","</div>\n","<div class='field-sets'>\n";Admin::get()->printSelectionColumns($N,$c);Admin::get()->printSelectionSearch($Z,$Dk,$t);Admin::get()->printSelectionOrder($D,$Ai,$t);Admin::get()->printSelectionLimit($w);Admin::get()->printSelectionLength($mm);Admin::get()->printSelectionAction($t);echo"</div>\n</form>\n";foreach((array)$_GET["where"]as$X){if($X["op"]=="SQL"&&!is_same_origin()){echo"<p class='error'>".lang(151)." ".lang(152)."\n";page_footer();exit;}}$E=isset($_GET["page"])?$_GET["page"]:null;if($E=="last"){$_e=Connection::get()->getValue(count_rows($a,$Z,$Wf,$Ne));$E=$w?(int)floor(max(0,intval($_e)-1)/$w):0;}else{$_e=false;$E=(int)$E;}$Ek=$Ba;$Oe=$Ne;if(!$Ek){$Ek[]="*";$ec=convert_fields($c,$l,$N);if($ec)$Ek[]=substr($ec,2);}foreach($N
as$u=>$X){$k=$l[idf_unescape($X)];if($k&&($Ma=convert_field($k)))$Ek[$u]="$Ma AS $X";}if(!$Wf&&$Um){foreach($Um
as$u=>$X){$Ek[]=idf_escape($u);if($Oe)$Oe[]=idf_escape($u);}}$J=Driver::get()->select($a,$Ek,$Z,$Oe,$D,$w,$E,true);if(!is_object($J))echo"<p class='error'>".(error()?:lang(123))."\n";else{if(DIALECT=="mssql"&&$E)$J->seek($w*$E);$M=[];while($L=$J->fetchAssoc()){if($E&&DIALECT=="oracle")unset($L["RNUM"]);$M[]=$L;}if($_GET["modify"]&&$M){$gh=max_input_vars(count($M[0])+1,20);echo($gh&&count($M)>$gh?"<p class='error'>".max_input_vars_error()."\n":"");}echo"<form id='selection_form' action='' method='post' enctype='multipart/form-data'>\n","<div class='table-footer-parent'>\n";if($_GET["page"]!="last"&&$w&&$Ne&&$Wf&&DIALECT=="sql")$_e=Connection::get()->getValue(" SELECT FOUND_ROWS()");$sd=false;if(!$M)echo"<p class='message'>".lang(90)."\n";else{$Xa=Admin::get()->getBackwardKeys($a,$Ul);echo"<div class='scrollable'>\n","<table id='table' class='nowrap checkable'>\n","<thead><tr>";if($Ne||!$N){echo"<th class='actions'><input type='checkbox' id='all-page' class='jsonly' title='".lang(290)."'>".script("gid('all-page').onclick = partial(formCheck, /^check/);","");if(Admin::get()->isDataEditAllowed())echo" <a href='",h($_GET["modify"]?remove_from_uri("modify"):$_SERVER["REQUEST_URI"]."&modify=1")."' title='",lang(291),"'>",icon_solo("edit-all"),"</a>";}$Lh=[];$bi=[];$Fe=[];reset($N);$Qj=1;foreach($M[0]as$u=>$X){if(!isset($Um[$u])){$Hk=key($N);$X=isset($_GET["columns"][$Hk])?$_GET["columns"][$Hk]:[];$k=$l[$N?($X?$X["col"]:current($N)):$u];$A=($k?Admin::get()->getFieldName($k,$Qj):(isset($X["fun"])?"*":h($u)));if($A!=""){$Qj++;$Lh[$u]=$A;$bi[$u]=($k&&preg_match(number_type(),$k["type"])&&!preg_match('~\[~',$k["full_type"]))||preg_match('~^(CHAR_LENGTH|ROUND|FLOOR|CEIL|UNIX_TIMESTAMP|TIME_TO_SEC|COUNT|SUM)\(~',(string)current($N));$b=idf_escape($u);$lf=remove_from_uri('(order|desc)[^=]*|page').'&order%5B0%5D='.urlencode($u);$Kc="&desc%5B0%5D=1";$_i=isset($D[0])?$D[0]:"";$hl=preg_replace('~ DESC( NULLS LAST)?$~','',$_i);$jl=($hl==$b||$hl==$u);echo"<th id='th[".h(bracket_escape($u))."]'".($bi[$u]?" class='number'":"").($jl?" aria-sort='".($hl==$_i?"ascending":"descending")."'":"").">";$Ee=apply_sql_function(isset($X["fun"])?$X["fun"]:null,$A);$il=isset($k["privileges"]["order"])||(isset($X["fun"])?$X["fun"]:null);if($il)echo'<a href="',h($lf.($jl&&$hl==$_i?$Kc:'')),'">',"$Ee</a>";else
echo$Ee;echo"<span class='column'>";if($il)echo"<a href='".h($lf.$Kc)."' title='".lang(63)."' class='button light'>",icon_solo("arrow-down"),"</a>";if(!isset($X["fun"])&&isset($k["privileges"]["where"]))echo"<a href='#fieldset-search' title='".lang(60)."' class='button light jsonly'>",icon_solo("search"),"</a>",script("qsl('a').onclick = partial(selectSearch, '".js_escape($u)."');");echo"</span>";}$Fe[$u]=isset($X["fun"])?$X["fun"]:null;next($N);}}$Fg=[];if($_GET["modify"]){foreach($M
as$L){foreach($L
as$u=>$X)$Fg[$u]=max($Fg[$u],min(40,strlen(utf8_decode($X))));}}if($Xa)echo"<th>".lang(19)."</th>";echo"</thead>\n","<tbody>\n";if(is_ajax())ob_end_clean();foreach(Admin::get()->fillForeignDescriptions($M,$te)as$Hh=>$L){$Qm=unique_array($M[$Hh],$t);if(!$Qm){$Qm=[];reset($N);foreach($M[$Hh]as$u=>$X){if(!preg_match('~^(COUNT|AVG|GROUP_CONCAT|MAX|MIN|SUM)\(~',current($N)))$Qm[$u]=$X;next($N);}}$Rm="";foreach($Qm
as$u=>$X){$k=isset($l[$u])?$l[$u]:null;$Vf=$k&&is_blob($k);if((DIALECT=="sql"||DIALECT=="pgsql")&&$k&&($Vf||preg_match('~char|text|enum|set~',$k["type"]))&&strlen($X)>64){$u=(strpos($u,'(')?$u:idf_escape($u));$u="MD5(".($Vf||DIALECT!='sql'||preg_match("~^utf8~",isset($k["collation"])?$k["collation"]:"")?$u:"CONVERT($u USING ".charset(Connection::get()).")").")";$X=md5($Vf?(string)Connection::get()->formatValue($X,$k):$X);}$Rm
.="&".($X!==null?urlencode("where[".bracket_escape($u)."]")."=".urlencode($X===false?"f":$X):"null%5B%5D=".urlencode($u));}echo"<tr>";if($Ne||!$N){echo"<td class='actions'>",checkbox("check[]",substr($Rm,1),in_array(substr($Rm,1),(array)$_POST["check"]));if(!$Wf&&Admin::get()->isDataEditAllowed())echo" <a href='",h(ME."edit=".urlencode($a).$Rm),"' class='edit' title='",lang(39),"'>",icon_solo("edit"),"</a>";}reset($N);foreach($L
as$u=>$X){if(isset($Lh[$u])){$b=current($N);$k=isset($l[$u])?$l[$u]:null;$x="";if($k&&is_blob($k)&&$X!="")$x=ME.'download='.urlencode($a).'&field='.urlencode($u).$Rm;if(!$x&&$X!==null){foreach((array)$te[$u]as$o){if(count($te[$u])==1||end($o["source"])==$u){$x="";foreach($o["source"]as$q=>$kl)$x
.=where_link($q,$o["target"][$q],$M[$Hh][$kl]);$x=($o["db"]!=""?preg_replace('~([?&]db=)[^&]+~','\1'.urlencode($o["db"]),ME):ME).'select='.urlencode($o["table"]).$x;if($o["ns"])$x=preg_replace('~([?&]ns=)[^&]+~','\1'.urlencode($o["ns"]),$x);if(count($o["source"])==1)break;}}}if($b=="COUNT(*)"){$x=ME."select=".urlencode($a);$q=0;foreach((array)$_GET["where"]as$W){if(!array_key_exists($W["col"],$Qm))$x
.=where_link($q++,$W["col"],$W["val"],$W["op"]);}foreach($Qm
as$hg=>$W)$x
.=where_link($q++,$hg,$W);}$Yh=$X===null;$mf=select_value($X,$x,$k,$mm);$Fd=bracket_escape($u);$r=h("val[$Rm][$Fd]");$wj=isset($_POST["val"][$Rm][$Fd])?$_POST["val"][$Rm][$Fd]:null;$Ym=isset($k["privileges"]["update"])?$k["privileges"]["update"]:false;$qd=!is_array($X)&&!($k&&is_blob($k))&&is_utf8((string)$X)&&$M[$Hh][$u]==$X&&!$Fe[$u]&&!(isset($k["generated"])?$k["generated"]:false);$U=($b&&preg_match('~^(AVG|MIN|MAX)\((.+)\)~',$b,$_)?$l[idf_unescape($_[2])]["type"]:(isset($k["type"])?$k["type"]:null));$Ah=$U=="money"||($b&&preg_match('~^SUM\((.+)\)~',$b,$_)&&$l[idf_unescape($_[1])]["type"])=="money";$km=$U&&preg_match('~text|json|lob~',$U);$Ab=$bi[$u]&&($Yh||is_numeric(strip_tags($mf))||$Ah)?"class='number'":"";echo"<td id='$r' $Ab";if(($_GET["modify"]&&$qd&&!$Yh)||$wj!==null){$sd=true;$Te=h($wj!==null?$wj:$X);echo" data-editing='true'>".($km?"<textarea name='$r' cols='30' rows='".(substr_count($X,"\n")+1)."'>$Te</textarea>":"<input class='input' name='$r' value='$Te' size='$Fg[$u]'>");}else{$Vg=strpos($mf,"<i>…</i>");if($Ym)echo" data-text='".($Vg?2:($km?1:0))."'".($qd?"":" data-warning='".lang(292)."'");echo">$mf";}}next($N);}if($Xa){echo"<td>";Admin::get()->printBackwardKeys($Xa,$M[$Hh]);echo"</td>";}echo"</tr>\n";}if(is_ajax())exit;echo"</tbody>\n",script("mixin(qs('#table tbody'), {onclick: event => tableClick(event, false, ".(Admin::get()->isDataEditAllowed()?"true":"false")."), ondblclick: event => tableClick(event, true), onkeydown: onEditingKeydown});"),"</table>\n",script("initToggles(gid('table'));"),"</div>\n";}if(!is_ajax()){if($M||$E){$Hd=true;if($_GET["page"]!="last"){if(!$w||(count($M)<$w&&($M||!$E)))$_e=($E?$E*$w:0)+count($M);elseif(DIALECT!="sql"||!$Wf){$_e=($Wf?false:found_rows($R,$Z));if($_e<max(1e4,2*($E+1)*$w))$_e=first(slow_query(count_rows($a,$Z,$Wf,$Ne)));elseif(DIALECT=='sql'||DIALECT=='pgsql')$Hd=false;}}$Ri=($w&&($_e===false||$_e>$w||$E));if($Ri){if(($_e===false?count($M)+1:$_e-$E*$w)>$w)echo'<p class="links">','<a href="',h(remove_from_uri("page")."&page=".($E+1)),'" class="loadmore">',icon("expand"),lang(293),'</a>',script("qsl('a').onclick = partial(loadNextPage, $w, '".js_escape(lang(294))."');","");echo"\n";}echo"<div class='table-footer'><div class='field-sets'>\n";if($Ri){$eh=($_e===false?$E+(count($M)>=$w?2:1):(int)floor(($_e-1)/$w));$bd="<li>…</li>";echo"<fieldset><legend>".lang(295)."</legend>";if(DIALECT!="simpledb"){echo"<div id='fieldset-pagination' class='fieldset-content'><ul class='pagination'>",pagination(0,$E);if($E>5)echo$bd;for($q=max(1,$E-4);$q<min($eh,$E+5);$q++)echo
pagination($q,$E);if($eh>0){if($E+5<$eh)echo$bd;echo($Hd&&$_e!==false?pagination($eh,$E):" <a href='".h(remove_from_uri("page")."&page=last")."' title='~$eh'>".lang(296)."</a>");}echo"</ul></div>";}else{echo"<div id='fieldset-pagination'><ul class='pagination'>",pagination(0,$E);if($E>1)echo$bd;if($E)echo
pagination($E,$E);if($eh>$E){echo
pagination($E+1,$E);if($eh>$E+1)echo$bd;}echo"</ul></div>";}echo"</fieldset>\n";}echo"<fieldset>","<legend>".lang(297)."</legend><div class='fieldset-content'>";$Uc=($Hd?"":"~ ").$_e;echo
checkbox("all",1,0,($_e!==false?($Hd?"":"~ ").lang(196,$_e):""),"countRows.call(this, '$Uc');")."\n","</div></fieldset>\n";if(Admin::get()->isDataEditAllowed()){echo"<fieldset",($_GET["modify"]?'':' class="jsonly"'),">","<legend>",lang(291),"</legend>";$uk=($_GET["modify"]?"":" data-inline-edit='1'".($sd?"":" disabled"));echo"<div class='fieldset-content'",($_GET["modify"]?"":" title='".lang(286)."'"),">","<input type='submit' class='button' id='modify-save' value='",lang(116),"'",$uk,">","</div>","</fieldset>\n","<fieldset>","<legend>",lang(166)," <span id='selected'></span></legend>","<div class='fieldset-content'>","<input type='submit' class='button' name='edit' value='",lang(39),"'> ","<input type='submit' class='button' name='clone' value='",lang(282),"'> ","<input type='submit' class='button' name='delete' value='",lang(120),"'>",confirm(),"</div>","</fieldset>\n";}$tc=Admin::get()->getDataDumpFormats();foreach((array)$_GET["columns"]as$b){if($b["fun"]){unset($tc['sql']);break;}}if($tc){print_fieldset_start("export",lang(75)." <span id='selected2'></span>","export");echo
html_select("format",$tc,$P->getParameter("exportFormat"));$Oi=Admin::get()->getDumpOutputs();echo($Oi?" ".html_select("output",$Oi,$P->getParameter("exportOutput")):"")," <input type='submit' class='button' name='export' value='".lang(75)."'>\n";print_fieldset_end("export");}echo"</div></div>\n",script("initTableFooter()");}echo"</div>\n";if(Admin::get()->isDataEditAllowed()){echo"<p>","<a href='#import'>",icon("import"),lang(74),"</a>",script("qsl('a').onclick = partial(toggle, 'import');",""),"</p>","<p id='import'",($_POST["import"]?"":" class='hidden'"),">";if(ini_bool("file_uploads"))echo"<input type='file' name='csv_file'> ",html_select("import_format",["csv"=>"CSV,","csv;"=>"CSV;","tsv"=>"TSV"],$P->getParameter("exportFormat"))," <input type='submit' class='button default' name='import' value='".lang(74)."'>",file_upload_form_script("selection_form","csv_file");else
echo
lang(203);echo"</p>";}echo
input_token(),"</form>\n",(!$Ne&&$N?"":script("tableCheck();"));}else
echo"</div>\n";}}if(is_ajax()){ob_end_clean();exit;}}elseif(isset($_GET["variables"])){$vl=isset($_GET["status"]);$T=$vl?lang(159):lang(158);page_header($T,[$T]);$on=($vl?Admin::get()->getStatusVariables():Admin::get()->getServerVariables());if(!$on)echo"<p class='message'>",lang(90),"</p>\n";else{echo"<div class='scrollable'><table>\n";foreach($on
as$L){echo"<tr>";$u=array_shift($L);echo"<th><code class='jush-".DIALECT.($vl?"status":"set")."'>".h($u)."</code></th>";foreach($L
as$X)echo"<td>",nl2br(h($X)),"</td>";echo"</tr>\n";}echo"</table></div>\n";}}elseif(isset($_GET["script"])){header("Content-Type: text/javascript; charset=utf-8");if($_GET["script"]=="db"){$Hl=["Data_length"=>0,"Index_length"=>0,"Data_free"=>0];$f=[];$yc=null;foreach(table_status()as$A=>$R){$f["Comment-$A"]=h($R["Comment"]);if(!is_view($R)||preg_match('~materialized~i',$R["Engine"])){$f["Engine-$A"]=h($R["Engine"]);$Eb=isset($R["Collation"])?$R["Collation"]:"";if($Eb==""){if($yc===null)$yc=(string)db_collation(DB,collations());$Eb=$yc;}$f["Collation-$A"]=h($Eb);foreach($Hl+["Auto_increment"=>0,"Rows"=>0]as$u=>$X){if($R[$u]!=""){$X=format_number($R[$u]);if($X>=0)$f["$u-$A"]=($u=="Rows"?format_rows($R):$X);if(isset($Hl[$u]))$Hl[$u]+=($R["Engine"]!="InnoDB"||$u!="Data_free"?$R[$u]:0);}elseif(array_key_exists($u,$R))$f["$u-$A"]="?";}}}if(function_exists('AdminNeo\db_status'))$Hl=db_status();foreach($Hl
as$u=>$X)$f["sum-$u"]=format_number($X);echo
json_encode($f,JSON_UNESCAPED_UNICODE);}elseif($_GET["script"]=="kill")Connection::get()->query("KILL ".number($_POST["kill"]));else{$f=[];foreach(count_tables(Admin::get()->getDatabases(false))as$h=>$X){$f["tables-$h"]=$X;$f["size-$h"]=db_size($h);}echo
json_encode($f,JSON_UNESCAPED_UNICODE);}exit;}else{$dm=array_merge((array)$_POST["tables"],(array)$_POST["views"]);if($dm&&!$_POST["search"]){$J=true;$nh="";if(DIALECT=="sql"&&$_POST["tables"]&&count($_POST["tables"])>1&&($_POST["drop"]||$_POST["truncate"]||$_POST["copy"]))queries("SET foreign_key_checks = 0");if($_POST["truncate"]){if($_POST["tables"])$J=truncate_tables($_POST["tables"]);$nh=lang(298);}elseif($_POST["move"]){$J=move_tables((array)$_POST["tables"],(array)$_POST["views"],$_POST["target"]);$nh=lang(299);}elseif($_POST["copy"]){$J=copy_tables((array)$_POST["tables"],(array)$_POST["views"],$_POST["target"]);$nh=lang(300);}elseif($_POST["drop"]){if($_POST["views"])$J=drop_views($_POST["views"]);if($J&&$_POST["tables"])$J=drop_tables($_POST["tables"]);$nh=lang(301);}elseif(DIALECT=="sqlite"&&$_POST["check"]){foreach((array)$_POST["tables"]as$Q){foreach(get_rows("PRAGMA integrity_check(".q($Q).")")as$L)$nh
.="<b>".h($Q)."</b>: ".h($L["integrity_check"])."<br>";}}elseif(DIALECT!="sql"){$J=(DIALECT=="sqlite"?queries("VACUUM"):apply_queries("VACUUM".($_POST["optimize"]?" ANALYZE":""),(array)$_POST["tables"]));$nh=lang(302);}elseif(!$_POST["tables"])$nh=lang(79);elseif($J=queries(($_POST["optimize"]?"OPTIMIZE":($_POST["check"]?"CHECK":($_POST["repair"]?"REPAIR":"ANALYZE")))." TABLE ".implode(", ",array_map('AdminNeo\idf_escape',$_POST["tables"])))){while($L=$J->fetchAssoc())$nh
.="<b>".h($L["Table"])."</b>: ".h($L["Msg_text"])."<br>";}queries_redirect($_SERVER["REQUEST_URI"],$nh,(bool)$J);}if($_GET["ns"]=="")page_header(lang(31).": ".h(DB),true);else
page_header(lang(191).": ".h($_GET["ns"]),true);Admin::get()->printDatabaseMenu();if($_GET["ns"]===""){echo"<h2 id='schemas'>".lang(303)."</h2>\n";$Ak=Admin::get()->getSchemas();if(!$Ak)echo"<p class='message'>".lang(304)."\n";else{echo"<div class='scrollable'>\n","<table class='nowrap'>\n",'<thead><tr class="wrap"><th>',lang(191),"</th></tr></thead>";foreach($Ak
as$A)echo"<tr><th><a href='",h(ME),"ns=".urlencode($A),"' title='",lang(305),"'>".h($A)."</a></th></tr>";echo'</table></div>';}echo'<p class="links"><a href="'.h(ME).'scheme=">'.icon("database-add").lang(77)."</a>\n";}else{echo"<h2 id='tables-views'>".lang(306)."</h2>\n";$Yl=['sql'=>'show-table-status.html','mariadb'=>'reference/sql-statements/administrative-sql-statements/show/show-table-status'];$yc=db_collation(DB,collations());$c=["Engine"=>["label"=>lang(170),"doc"=>doc_link(['sql'=>'storage-engines.html','mariadb'=>'server-usage/storage-engines']),],];if($yc!="")$c["Collation"]=["label"=>lang(47),"doc"=>doc_link(['sql'=>'charset-charsets.html','mariadb'=>'reference/data-types/string-data-types/character-sets/supported-character-sets-and-collations']),];$c+=["Data_length"=>["label"=>lang(307),"doc"=>doc_link($Yl+['pgsql'=>'functions-admin.html#FUNCTIONS-ADMIN-DBOBJECT','oracle'=>'REFRN20286']),"link"=>"create","title"=>lang(36),],"Index_length"=>["label"=>lang(308),"doc"=>doc_link($Yl+['pgsql'=>'functions-admin.html#FUNCTIONS-ADMIN-DBOBJECT']),"link"=>"indexes","title"=>lang(176),],"Data_free"=>["label"=>lang(309),"doc"=>doc_link($Yl),"link"=>"edit","title"=>lang(8),],"Auto_increment"=>["label"=>lang(49),"doc"=>doc_link(['sql'=>'example-auto-increment.html','mariadb'=>'reference/data-types/auto_increment']),"link"=>"auto_increment=1&create","title"=>lang(36),],"Rows"=>["label"=>lang(310),"doc"=>doc_link($Yl+['pgsql'=>'catalog-pg-class.html#CATALOG-PG-CLASS','oracle'=>'REFRN20286']),"link"=>"select","title"=>lang(34),],];if(support("comment"))$c["Comment"]=["label"=>lang(48),"doc"=>doc_link($Yl+['pgsql'=>'functions-info.html#FUNCTIONS-INFO-COMMENT-TABLE']),];$D=(is_string($_GET["order"])?$_GET["order"]:"");$Lc=null;if(preg_match('~^(.+)-(asc|desc)$~',$D,$z)){$D=$z[1];$Lc=($z[2]=="desc");}if($D!="__table"&&!isset($c[$D]))$D="";if($Lc===null)$Lc=isset($c[$D]["link"]);$In=($D!=""&&$D!="__table")||support("fast_status");$bm=($In?table_status():tables_list());if(!$bm)echo"<p class='message'>".lang(79)."\n";else{echo"<form action='' method='post'>\n","<div class='table-footer-parent'>\n";if(support("table")){echo"<div class='field-sets'>\n","<fieldset><legend>".lang(311)." <span id='selected2'></span></legend><div class='fieldset-content'>",html_select("op",Admin::get()->getOperators(),isset($_POST["op"])?$_POST["op"]:Driver::get()->getLikeOperator()),"<input type='search' class='input' name='query' value='".h($_POST["query"])."'>",script("qsl('input').onkeydown = event => bodyKeydown(event, 'search');","")," <input type='submit' class='button' name='search' value='".lang(60)."'>\n","</div></fieldset>\n","</div>\n";if($_POST["search"]&&$_POST["query"]!=""){$_GET["where"][0]["op"]=$_POST["op"];search_tables();}}echo"<div class='scrollable'>\n","<table class='nowrap checkable'>\n",'<thead><tr class="wrap">','<th class="actions"><input id="check-all" type="checkbox" class="input jsonly" title="'.lang(190).'">'.script("gid('check-all').onclick = partial(formCheck, /^(tables|views)\[/);","");$Ih=($D==""||$D=="__table");$Tl=($Ih&&!$Lc?ME."order=__table-desc":substr(ME,0,-1));$Kh=($Ih&&DIALECT!="sqlite");echo'<th'.($Kh?" aria-sort='".($Lc?"descending":"ascending")."'":'').'><a href="'.h($Tl).'">'.lang(9).'</a>';foreach($c
as$u=>$b){$Oc=($u===$D?!$Lc:isset($b["link"]));echo'<th'.($u===$D?" aria-sort='".($Lc?"descending":"ascending")."'":'').'><a href="'.h(ME)."order=$u-".($Oc?"desc":"asc").'">'.$b["label"].'</a>'.$b["doc"];}echo"</thead>\n","<tbody>\n";if($D=="__table"){if($Lc)$bm=array_reverse($bm,true);}elseif($D){uasort($bm,function($sa,$Ua)use($D,$Lc){$Jn=isset($sa[$D])?$sa[$D]:null;$Kn=isset($Ua[$D])?$Ua[$D]:null;$J=($Jn<$Kn?-1:($Jn>$Kn?1:0));return($Lc?-$J:$J);});}$Hl=["Data_length"=>0,"Index_length"=>0,"Data_free"=>0];$S=0;foreach($bm
as$A=>$vl){$sn=($In?is_view($vl):$vl!==null&&!preg_match('~table|sequence~i',$vl));$yd=($In?(isset($vl["Engine"])?$vl["Engine"]:""):$vl);$r=h("Table-".$A);echo'<tr><th class="actions">'.checkbox(($sn?"views[]":"tables[]"),$A,in_array("$A",$dm,true),"","","",$r);if(!Admin::get()->getSettings()->isSelectionPreferred()&&(support("table")||support("indexes")))$ua="table";else$ua="select";echo"<th><a href='",h(ME),"$ua=",urlencode($A),"' id='$r'>",h($A),"</a></th>";if($sn&&!preg_match('~materialized~i',$yd)){$T=lang(169);$Kb=count($c)-(support("comment")?2:1);echo'<td colspan="'.$Kb.'">'.(support("view")?"<a href='".h(ME)."view=".urlencode($A)."' title='".lang(37)."'>$T</a>":$T),"<td align='right'><a href='".h(ME)."select=".urlencode($A)."' title='".lang(34)."'>?</a>";}else{foreach($c
as$u=>$b){if($u=="Comment")continue;$r=" id='$u-".h($A)."'";$x=isset($b["link"])?$b["link"]:"";if(!$x){$X="";if($In){$X=isset($vl[$u])?$vl[$u]:"";if($u=="Collation"&&$X=="")$X=$yc;}echo"<td$r>".h($X);continue;}$X="?";if($In){$B=isset($vl[$u])?$vl[$u]:"";if(is_numeric($B)&&$B>=0){$X=($u=="Rows"?format_rows($vl):format_number($B));if(isset($Hl[$u])&&($yd!="InnoDB"||$u!="Data_free"))$Hl[$u]+=$B;}}echo"<td align='right'>".(support("table")||$u=="Rows"||(support("indexes")&&$u!="Data_length")?"<a href='".h(ME."$x=").urlencode($A)."'$r title='".$b["title"]."'>".h($X)."</a>":"<span$r>".h($X)."</span>");}$S++;}echo(support("comment")?"<td id='Comment-".h($A)."'>".($In?h(isset($vl["Comment"])?$vl["Comment"]:""):""):""),"\n";}echo"</tbody>\n",script("mixin(qsl('tbody'), {onclick: tableClick, ondblclick: event => tableClick(event, true)});"),"<tfoot><tr>","<td><th>".lang(283,count($bm)),"<td>".h(DIALECT=="sql"?Connection::get()->getValue("SELECT @@default_storage_engine"):""),($yc!=""?"<td>".h($yc):"");if($In&&function_exists('AdminNeo\db_status'))$Hl=db_status();foreach($Hl
as$u=>$Gl)echo"<td align='right' id='sum-$u'>".($In?format_number($Gl):"");echo"<td></td><td></td>";if(support("comment"))echo"<td></td>";echo"</tr></tfoot>\n","</table>\n","</div>\n",($In?"":script("ajaxSetHtml('".js_escape(ME)."script=db');"));if(Admin::get()->isDataEditAllowed()){echo"<div class='table-footer'><div class='field-sets'>\n";$ln="<input type='submit' class='button' value='".lang(312)."'> ".help_script("VACUUM");$wi="<input type='submit' class='button' name='optimize' value='".lang(313)."'> ".help_script(DIALECT=="sql"?"OPTIMIZE TABLE":"VACUUM ANALYZE");echo"<fieldset><legend>".lang(166)." <span id='selected'></span></legend><div class='fieldset-content'>".(DIALECT=="sqlite"?$ln."<input type='submit' class='button' name='check' value='".lang(314)."'> ".help_script("PRAGMA integrity_check"):(DIALECT=="pgsql"?$ln.$wi:(DIALECT=="sql"?"<input type='submit' class='button' value='".lang(315)."'> ".help_script("ANALYZE TABLE").$wi."<input type='submit' class='button' name='check' value='".lang(314)."'> ".help_script("CHECK TABLE")."<input type='submit' class='button' name='repair' value='".lang(316)."'> ".help_script("REPAIR TABLE"):"")))."<input type='submit' class='button' name='truncate' value='".lang(317)."'> ".help_script(DIALECT=="sqlite"?"DELETE":("TRUNCATE".(DIALECT=="pgsql"?"":" TABLE"))).confirm()."<input type='submit' class='button' name='drop' value='".lang(167)."'>".help_script("DROP TABLE").confirm()."\n";$g=(support("scheme")?Admin::get()->getSchemas():Admin::get()->getDatabases());echo"</div></fieldset>\n";if(count($g)!=1&&DIALECT!="sqlite"){echo"<fieldset><legend>".lang(318)." <span id='selected3'></span></legend><div>";$h=(isset($_POST["target"])?$_POST["target"]:(support("scheme")?$_GET["ns"]:DB));echo($g?html_select("target",$g,$h,"","label-move"):'<input class="input" name="target" value="'.h($h).'" autocapitalize="off">')," <input type='submit' class='button' name='move' value='".lang(319)."'>",(support("copy")?" <input type='submit' class='button' name='copy' value='".lang(320)."'> ".checkbox("overwrite",1,$_POST["overwrite"],lang(321)):""),"</div></fieldset>\n";}echo
input_hidden("all"),script("qsl('input').onclick = partial(countTables, $S);"),input_token(),"</div></div>\n",script("initTableFooter()");}echo"</div>\n","</form>\n",script("tableCheck();");}echo'<p class="links"><a href="',h(ME),'create=">',icon("table-add"),lang(78),"</a>\n";if(support("view"))echo'<a href="',h(ME),'view=">',icon("view-add"),lang(246),"</a>\n";if(support("routine")){echo"<h2 id='routines'>".lang(186)."</h2>\n";$qk=routines();if($qk){$Qb=$qk[0]["ROUTINE_COMMENT"]!==null;echo"<table>\n",'<thead><tr>','<th>',lang(226),'</th><td>',lang(45),'</td><td>',lang(263),"</td>";if($Qb)echo"<td>",lang(48),"</td>";echo"<td></td>","</tr></thead>\n";foreach($qk
as$L){$A=($L["SPECIFIC_NAME"]==$L["ROUTINE_NAME"]?"":"&name=".urlencode($L["ROUTINE_NAME"]));echo'<tr>','<th><a href="',h(ME.($L["ROUTINE_TYPE"]!="PROCEDURE"?'callf=':'call=').urlencode($L["SPECIFIC_NAME"]).$A),'">',h($L["ROUTINE_NAME"]),'</a></th>','<td>',h($L["ROUTINE_TYPE"]),'</td>','<td>',h($L["DTD_IDENTIFIER"]),'</td>';if($Qb)echo'<td>',truncate_utf8(preg_replace('~\s{2,}~'," ",trim($L["ROUTINE_COMMENT"])),50),'</td>';echo'<td><a href="'.h(ME.($L["ROUTINE_TYPE"]!="PROCEDURE"?'function=':'procedure=').urlencode($L["SPECIFIC_NAME"]).$A).'">'.lang(179)."</a></td>";}echo"</table>\n";}echo'<p class="links">';if(support("procedure"))echo'<a href="',h(ME),'procedure=">',icon("function-add"),lang(262),"</a>";echo'<a href="',h(ME),'function=">',icon("function-add"),lang(261),"</a>\n","</p>\n";}if(support("event")){echo"<h2 id='events'>".lang(187)."</h2>\n";$M=get_rows("SHOW EVENTS");if($M){echo"<table>\n","<thead><tr><th>".lang(226)."<td>".lang(322)."<td>".lang(252)."<td>".lang(253)."<td></thead>\n";foreach($M
as$L)echo"<tr>","<th>".h($L["Name"]),"<td>".($L["Execute at"]?lang(323)."<td>".h($L["Execute at"]):lang(254)." ".h($L["Interval value"])." ".h($L["Interval field"])."<td>".h($L["Starts"])),"<td>".h($L["Ends"]),'<td><a href="'.h(ME).'event='.urlencode($L["Name"]).'">'.lang(179).'</a>';echo"</table>\n";$Gd=Connection::get()->getValue("SELECT @@event_scheduler");if($Gd&&$Gd!="ON")echo"<p class='error'><code class='jush-sqlset'>event_scheduler</code>: ".h($Gd)."\n";}echo'<p class="links"><a href="',h(ME),'event=">',icon("event-add"),lang(251),"</a></p>\n";}}}page_footer();