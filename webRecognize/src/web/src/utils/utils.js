import moment from "moment";

function groupsToLowerCase(groups) {
    [...groups].forEach((group) => {
        group.elements.forEach((el) => {
            el.target = el.target.toLowerCase();
        });

        if (group.groups) {
            groupsToLowerCase(group.groups);
        }
    });
}

function elementsGroupsToLowerCase(elements) {
    let el = elements;
    groupsToLowerCase(el.camera.groups);
    groupsToLowerCase(el.program.groups);
    return el;
}

const parseStringToDate = (datestring, return_moment = false) => {
    var parsed = moment(datestring, "DD_MM_YYYY_hh_mm_ss");
    return return_moment ? parsed : parsed.toDate();
};

const prepareNotifications = (notifications) =>
    notifications.map((not) => {
        not.datetime = parseStringToDate(not.datetime);
        if (not.type === "video" || not.type === "image") {
            not.content += `?directory=${encodeURIComponent(not.directory)}`;
        }

        return not;
    });

export default {
    elementsGroupsToLowerCase,
    prepareNotifications,
};
