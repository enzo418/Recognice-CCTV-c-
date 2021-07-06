import ElementController from '../utils/element_controller'

class NotificationPaginator extends ElementController {
    
    /**
     * 
     * @param {Array<any>} elements 
     * @param {HTMLElement} container 
     * @param {HTMLElement} elementPreviousLeft Element that as text has a counter of how many previous notifications are left
     * @param {HTMLElement} elementNextLeft Element that as text has a counter of how many following notifications are left
     */
    constructor (elements, container, elementPreviousLeft, elementNextLeft) {
        super(container);
        this.elements = elements;
        this.elementCounterPrevious = elementPreviousLeft;
        this.elementCounterNext = elementNextLeft;
    }

    setIndex(index) {
        this.index = index;
    }

    setElemets(elements) {
        this.elements = elements;
    }

    addElement(element) {
        this.elements.push(element);
        this.updateNotificationsNumber();
    }

    getElements() {
        return this.elements;
    }

    getElement(index) {
        return this.element[index];
    }

    getIndex() {
        return this.index;
    }

    updateNotificationsNumber() {
        this.elementCounterNext.innerText = "" + (this.elements.length - 1 - this.index);
        this.elementCounterPrevious.innerText = "" + this.index;
    }
    
    // updates the current index to the next one and changes the current notification
    nextNotification() {
        var i = this.index < this.elements.length - 1 ? this.index + 1 : 0;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }
    
    // updates the current index to the previous and changes the current displayed
    previousNotification() {
        var i = this.index > 0 ? this.index - 1 : this.elements.length - 1;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    // go to the start or end of the notification collection
    gotoUttermost(end = true) {
        var i = end ? this.elements.length - 1 : 0;
        this.gotoIndex(i);
        this.scrollContainerToElementTop();
    }

    /**
     * since all notification elements have their src attribute blank to save 
     * some memory when they are not displayed, this function sets each of 
     * them to the true src so that it can display the video or image to the user
     * @param {Object} elm an element from the elements array of this class 
     */
    setActualSources(elm) {
        if (elm.id) elm = elm.root[0];
    
        const loadVideos = function () {
            var videos = elm.getElementsByTagName("video");
            if (videos.length > 0) {
                [...videos].forEach(vid => {
                    if (vid.src === window.location.href) {
                        console.log("video:", { dataset: vid.dataset, video: vid, src: vid.dataset.videosrc })
                        vid.src = vid.dataset.videosrc;
                    }
                });
            }
        };
    
        if (elm.getElementsByTagName("img").length > 0) {
            var img = elm.getElementsByTagName("img")[0];
    
            // set src for the image elements
            img.src = img.dataset.src;
    
            // set src for the video elements
            img.onload = loadVideos;
        } else {
            loadVideos();
        }
    }

    /**
     * Changes the current notification
     * @param {number} index target notification
     */
    gotoIndex(index) {
        // remove all the sources from the images of all childrends of the notification container
        [...this.element.children].forEach(el => {
            var img = el.getElementsByTagName("img")[0];
            if (img) {
                img.dataset.src = img.src;
                img.src = "";
            }

            var videos = el.getElementsByTagName("video");
            if (videos.length > 0) {
                [...videos].forEach(vid => {
                    vid.dataset.videosrc = vid.src;
                    vid.src = "";
                });
            }
        });

        this.removeAllChild();
    
        var el = this.elements[index];
        this.setActualSources(el);
    
        // append notification
        this.element.appendChild(el.root[0]);

        // update current index
        this.index = index;

        // update before/after notifications left
        this.updateNotificationsNumber();
    }
}

export default NotificationPaginator;