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

export default {
    elementsGroupsToLowerCase,
};
