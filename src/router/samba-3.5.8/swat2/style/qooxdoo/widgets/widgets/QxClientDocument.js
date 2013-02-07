/* Copyright (c): 2002-2005 (Germany): United Internet, 1&1, GMX, Schlund+Partner, Alturo */
function QxClientDocument(clientWindow){QxWidget.call(this);this._window=clientWindow;this._document=this._window.getElement().document;this.setElement(this._document.body);this._lastBodyWidth=this._document.body.offsetWidth;this._lastBodyHeight=this._document.body.offsetHeight;this.addEventListener("resize",this._onresize);this.setTheme(this._themes[0]);this.setTabIndex(1);this._blocker=new QxBlocker;this._modalWidgets=[];this._modalNativeWindow=null;this._blocker.addEventListener("mousedown",function(e){if(this._modalNativeWindow){this._modalNativeWindow.focus();};},this);this._blocker.addEventListener("mouseup",function(e){if(this._modalNativeWindow){this._modalNativeWindow.focus();};},this);};QxClientDocument.extend(QxWidget,"QxClientDocument");QxClientDocument.addProperty({name:"theme",type:String});QxClientDocument.addProperty({name:"globalCursor",type:String});proto._renderInitialDone_horizontal=true;proto._renderInitialDone_vertical=true;proto._childOuterWidthChanged=function(vModifiedChild,_e5){};proto._childOuterHeightChanged=function(vModifiedChild,_e5){};proto._modifyParent=function(){return true;};proto._modifyVisible=function(){return true;};proto._modifyElement=function(_b1,_b2,_b3,_b4){if(!_b1){throw new Error("QxClientDocument does not accept invalid elements!");};_b1._QxWidget=this;this._applyStyleProperties(_b1,_b4);this._applyHtmlProperties(_b1,_b4);this._applyHtmlAttributes(_b1,_b4);this.setVisible(true,_b4);return true;};proto.getWindow=function(){return this._window;};proto.getTopLevelWidget=function(){return this;};proto.getDocumentElement=function(){return this._document;};proto.getEventManager=function(){return this._window.getEventManager();};proto.getFocusManager=function(){return this._window.getFocusManager();};proto._createElement=proto.createElementWrapper=function(){return true;};proto.isCreated=function(){return true;};proto.isFocusRoot=function(){return true;};proto.getFocusRoot=function(){return this;};proto.getToolTip=function(){return null;};proto.getParent=function(){return null;};proto.canGetFocus=function(){return true;};proto._visualizeBlur=function(){};proto._visualizeFocus=function(){};proto.block=function(activeWidget){this.add(this._blocker);if(typeof QxWindow=="function"&&activeWidget instanceof QxWindow){this._modalWidgets.push(activeWidget);var o=activeWidget.getZIndex();this._blocker.setZIndex(o);activeWidget.setZIndex(o+1);}else if(activeWidget instanceof QxNativeWindow){this._modalNativeWindow=activeWidget;this._blocker.setZIndex(1e7);};};proto.release=function(activeWidget){if(activeWidget){if(activeWidget instanceof QxNativeWindow){this._modalNativeWindow=null;}else {this._modalWidgets.remove(activeWidget);};};var l=this._modalWidgets.length;if(l==0){this.remove(this._blocker);}else {var oldActiveWidget=this._modalWidgets[l-1];var o=oldActiveWidget.getZIndex();this._blocker.setZIndex(o);oldActiveWidget.setZIndex(o+1);};};if((new QxClient).isMshtml()){proto._modifyGlobalCursor=function(_b1,_b2,_b3,_b4){var s=this._cursorStyleSheetElement;if(!s){s=this._cursorStyleSheetElement=this._document.createStyleSheet();};s.cssText=isValidString(_b1)?"*{cursor:"+_b1+" !important}":"";return true;};}else {proto._modifyGlobalCursor=function(_b1,_b2,_b3,_b4){var s=this._cursorStyleSheetElement;if(!s){s=this._cursorStyleSheetElement=this._document.createElement("style");s.type="text/css";this._document.getElementsByTagName("head")[0].appendChild(s);};var sheet=s.sheet;var l=sheet.cssRules.length;for(var i=l-1;i>=0;i--){sheet.deleteRule(i);};if(isValidString(_b1)){sheet.insertRule("*{cursor:"+_b1+" !important}",0);};return true;};};proto._onresize=function(e){if(typeof QxPopupManager=="function"){(new QxPopupManager).update();};var w=this._document.body.offsetWidth;var h=this._document.body.offsetHeight;if(this._lastBodyWidth!=w){this._lastBodyWidth=w;this._innerWidthChanged();};if(this._lastBodyHeight!=h){this._lastBodyHeight=h;this._innerHeightChanged();};};proto._themes=["Win9x","WinXP"];proto.getThemes=function(){return this._themes;};proto.registerTheme=function(v){if(this._themes.contains(v)){return;};this._themes.push(v);};proto.deregisterTheme=function(v){if(this.getTheme()==v){throw new Error("Could not remove currently selected theme!");};this._themes.remove(v);return true;};proto._modifyTheme=function(_b1,_b2,_b3,_b4){var vClass=this.getCssClassName();if(_b2){vClass=vClass.remove("QxTheme"+_b2," ");};if(_b1){vClass=vClass.add("QxTheme"+_b1," ");};this.setCssClassName(vClass);return true;};proto.add=function(){var a=arguments;var l=a.length;var t=a[l-1];if(typeof QxInline=="function"&&l>1&&typeof t=="string"){for(var i=0;i<l-1;i++){if(a[i]instanceof QxInline){a[i].setInlineNodeId(t);};a[i].setParent(this);};return this;}else {return QxWidget.prototype.add.apply(this,arguments);};};proto._getParentNodeForChild=function(otherObject){if(typeof QxInline=="function"&&otherObject instanceof QxInline){var inlineNodeId=otherObject.getInlineNodeId();if(isValid(inlineNodeId)){var inlineNode=document.getElementById(inlineNodeId);if(inlineNode){return inlineNode;};};throw new Error("Couldn't find target element for:"+otherObject);};return this.getElement();};proto.dispose=function(){if(this.getDisposed()){return;};this._window=this._document=null;QxWidget.prototype.dispose.call(this);return true;};