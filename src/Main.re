open Github;
open FetchResponse;

[@bs.val] external scrollTo : (int, int) => unit = "";


type action =
  | Init
  | GetRepositories(bool)
  | GotRepositories(bool, fetchResponse(array(repository)))
  | GetStargazers
  | GotStargazers(fetchResponse(array(stargazer)))
  | SetSelected(repository)
  | SetOrgname(string)
  ;

type state = {
    orgname: string,
    repositories: option(fetchResponse(array(repository))),
    stargazers: option(fetchResponse(array(stargazer))),
    selectedRepo: option(repository),
    loadingRepos: bool,
    loadingStargazers: bool,
    stargazersPanelRef: ref(option(Dom.element))
};


let mkReducer = (~client=defaultClient, ()) => action => {
    let getRepositories = self => self . ReasonReact.send(GetRepositories(false));
    switch (action) {
    | Init => (_state => ReasonReact.SideEffects(getRepositories));
    
    | GetRepositories(append) => (state =>
        ReasonReact.UpdateWithSideEffects({...state, loadingRepos:true}, self => {
            if (String.length(state.orgname) > 0) {
                let url =
                    switch(state.repositories) {
                    | Some(Success(_,Some(next))) => next
                    | _ => orgReposUrl(state.orgname)
                    };
                Js.Promise.(
                    client.fetchOrgRepositories(url)
                    |> then_(result => self.send(GotRepositories(append, result)) |> resolve)
                    |> ignore
                )
            }
        }))

    | GotRepositories(append, result) => (state => {
        let repositories = Some(switch(result) {
            | Success(repos,nextPageUrl) when append =>
                switch(state.repositories) {
                | Some (Success(curRepos,_)) => Success(Array.append(curRepos, repos), nextPageUrl);
                | _ => result;
                }
            | _ => result;
            });
        ReasonReact.Update({...state, loadingRepos:false, repositories});
    });

    | GetStargazers => (state =>
        switch(state.selectedRepo) {
        | Some(repo) =>
            ReasonReact.UpdateWithSideEffects({...state, loadingStargazers:true}, self => {
                Js.Promise.(
                    fetchStargazers(repo.stargazerUrl)
                    |> then_(result => self.send(GotStargazers(result)) |> resolve)
                    |> ignore
                )
            })
        | None => ReasonReact.NoUpdate
        })

    | GotStargazers(stargazers) => (state =>
        ReasonReact.UpdateWithSideEffects(
            { ...state
            , loadingStargazers:false
            , stargazers:Some(stargazers)
            }, (_self =>
                switch((stargazers, state.stargazersPanelRef^)) {
                | (Success(_,_), Some(r)) => scrollTo(0, ReactDOMRe.domElementToObj(r)##offsetTop)
                | _  => ()
                })
            )
        );

    | SetOrgname(orgname) => (state =>
        ReasonReact.UpdateWithSideEffects({...state, orgname, repositories:None}, getRepositories));

    | SetSelected(repo) => (state =>
        ReasonReact.UpdateWithSideEffects(
            {...state, selectedRepo:Some(repo)},
            (self => self.send(GetStargazers)))
        );
    }
}

let component = ReasonReact.reducerComponent("Main");

let make = (~orgname, ~client=defaultClient, ~maxStargazers=20, _children) => {
    ...component,

    didMount: self => self.send(Init),

    initialState: () => {
        orgname,
        repositories: None,
        stargazers: None,
        selectedRepo: None,
        loadingRepos: false,
        loadingStargazers: false,
        stargazersPanelRef: ref(None)

    },

    reducer: mkReducer(~client, ()),

    render: ({state,send,handle}) => ReasonReact.({
        let reposColumn =
            <div className="repositories column">
                <h2 className="header">
                    (string(state.orgname ++ "'s repositories"))
                </h2>
                <FetchResponse.Switch
                    value=state.repositories
                    successComp=(repositories =>
                        <Repositories repositories
                            onStargazersClick=(repo =>
                                send(SetSelected(repo)))
                            />)
                    notFoundComp=(
                        <Message.Error
                            message=string("No such organization: " ++ state.orgname)
                            header=string("Not found")
                            />)
                    onClickNext=Some(()=>send(GetRepositories(true)))
                />
            </div>;
        let setStargazersPanelRef = handle((ref,{state}) =>
            state.stargazersPanelRef := Js.Nullable.toOption(ref));
        let stargazersColumn =
            switch(state.selectedRepo) {
            | Some(repo) =>
                <div className="stargazers column" ref=setStargazersPanelRef>
                    <h2 className="header">
                        (string(repo.fullName ++ "'s stargazers"))
                    </h2>
                    <FetchResponse.Switch
                        value=state.stargazers
                        successComp=(stargazers => <Stargazers stargazers maxStargazers />)
                    />
                </div>
            | None => <div className="column" />
            };

        <div className="ui container">
            <TextInput
                value=state.orgname
                onChange=(orgname => (send(SetOrgname(orgname))))
                />
            <div className="ui segment">
                <div className="ui two column stackable grid layout">
                    reposColumn
                    stargazersColumn
                </div>
            </div>
        </div>
    })

}